/**
 * @file animationinfo.cpp
 *
 * Contains the core animation information and related logic
 */

#include "animationinfo.h"
#include "appfat.h"
#include "nthread.h"
#include "utils/log.hpp"
#include "utils/stdcompat/algorithm.hpp"

namespace devilution {

int8_t AnimationInfo::getFrameToUseForRendering() const
{
	// Normal logic is used,
	// - if no frame-skipping is required and so we have exactly one Animationframe per game tick
	// or
	// - if we load from a savegame where the new variables are not stored (we don't want to break savegame compatiblity because of smoother rendering of one animation)
	if (relevantFramesForDistributing_ <= 0)
		return std::max<int8_t>(0, currentFrame);

	if (currentFrame >= relevantFramesForDistributing_)
		return currentFrame;

	float ticksSinceSequenceStarted = ticksSinceSequenceStarted_;
	if (ticksSinceSequenceStarted_ < 0) {
		ticksSinceSequenceStarted = 0.0F;
		Log("getFrameToUseForRendering: Invalid ticksSinceSequenceStarted_ {}", ticksSinceSequenceStarted_);
	}

	// we don't use the processed game ticks alone but also the fraction of the next game tick (if a rendering happens between game ticks). This helps to smooth the animations.
	float totalTicksForCurrentAnimationSequence = getProgressToNextGameTick() + ticksSinceSequenceStarted;

	int8_t absoluteAnimationFrame = static_cast<int8_t>(totalTicksForCurrentAnimationSequence * tickModifier_);
	if (skippedFramesFromPreviousAnimation_ > 0) {
		// absoluteAnimationFrames contains also the Frames from the previous Animation, so if we want to get the current Frame we have to remove them
		absoluteAnimationFrame -= skippedFramesFromPreviousAnimation_;
		if (absoluteAnimationFrame < 0) {
			// We still display the remains of the previous Animation
			absoluteAnimationFrame = numberOfFrames + absoluteAnimationFrame;
		}
	} else if (absoluteAnimationFrame >= relevantFramesForDistributing_) {
		// this can happen if we are at the last frame and the next game tick is due (gfProgressToNextGameTick >= 1.0f)
		if (absoluteAnimationFrame >= (relevantFramesForDistributing_ + 1)) {
			// we should never have +2 frames even if next game tick is due
			Log("getFrameToUseForRendering: Calculated an invalid Animation Frame (Calculated {} MaxFrame {})", absoluteAnimationFrame, relevantFramesForDistributing_);
		}
		return relevantFramesForDistributing_ - 1;
	}
	if (absoluteAnimationFrame < 0) {
		Log("getFrameToUseForRendering: Calculated an invalid Animation Frame (Calculated {})", absoluteAnimationFrame);
		return 0;
	}
	return absoluteAnimationFrame;
}

float AnimationInfo::getAnimationProgress() const
{
	float ticksSinceSequenceStarted = ticksSinceSequenceStarted_;
	float tickModifier = tickModifier_;

	if (relevantFramesForDistributing_ <= 0) {
		// This logic is used if animation distribution is not active (see getFrameToUseForRendering).
		// In this case the variables calculated with animation distribution are not initialized and we have to calculate them on the fly with the given information.
		ticksSinceSequenceStarted = static_cast<float>((currentFrame * ticksPerFrame) + tickCounterOfCurrentFrame);
		tickModifier = 1.0F / static_cast<float>(ticksPerFrame);
	}

	float totalTicksForCurrentAnimationSequence = getProgressToNextGameTick() + ticksSinceSequenceStarted;
	float progressInAnimationFrames = totalTicksForCurrentAnimationSequence * tickModifier;
	float animationFraction = progressInAnimationFrames / static_cast<float>(numberOfFrames);
	return animationFraction;
}

void AnimationInfo::setNewAnimation(OptionalCelSprite celSprite, int8_t numberOfFrames, int8_t ticksPerFrame, AnimationDistributionFlags flags /*= AnimationDistributionFlags::None*/, int8_t numSkippedFrames /*= 0*/, int8_t distributeFramesBeforeFrame /*= 0*/, float previewShownGameTickFragments /*= 0.F*/)
{
	if ((flags & AnimationDistributionFlags::RepeatedAction) == AnimationDistributionFlags::RepeatedAction && distributeFramesBeforeFrame != 0 && this->numberOfFrames == numberOfFrames && currentFrame + 1 >= distributeFramesBeforeFrame && currentFrame != this->numberOfFrames - 1) {
		// We showed the same Animation (for example a melee attack) before but truncated the Animation.
		// So now we should add them back to the new Animation. This increases the speed of the current Animation but the game logic/ticks isn't affected.
		skippedFramesFromPreviousAnimation_ = this->numberOfFrames - currentFrame - 1;
	} else {
		skippedFramesFromPreviousAnimation_ = 0;
	}

	if (ticksPerFrame <= 0) {
		Log("setNewAnimation: Invalid ticksPerFrame {}", ticksPerFrame);
		ticksPerFrame = 1;
	}

	this->celSprite = celSprite;
	this->numberOfFrames = numberOfFrames;
	currentFrame = numSkippedFrames;
	tickCounterOfCurrentFrame = 0;
	this->ticksPerFrame = ticksPerFrame;
	ticksSinceSequenceStarted_ = 0.F;
	relevantFramesForDistributing_ = 0;
	tickModifier_ = 0.0F;
	isPetrified = false;

	if (numSkippedFrames != 0 || flags != AnimationDistributionFlags::None) {
		// Animation Frames that will be adjusted for the skipped Frames/game ticks
		int8_t relevantAnimationFramesForDistributing = numberOfFrames;
		if (distributeFramesBeforeFrame != 0) {
			// After an attack hits (_pAFNum or _pSFNum) it can be canceled or another attack can be queued and this means the animation is canceled.
			// In normal attacks frame skipping always happens before the attack actual hit.
			// This has the advantage that the sword or bow always points to the enemy when the hit happens (_pAFNum or _pSFNum).
			// Our distribution logic must also regard this behaviour, so we are not allowed to distribute the skipped animations after the actual hit (_pAnimStopDistributingAfterFrame).
			relevantAnimationFramesForDistributing = distributeFramesBeforeFrame - 1;
		}

		// Game ticks that will be adjusted for the skipped Frames/game ticks
		int8_t relevantAnimationTicksForDistribution = relevantAnimationFramesForDistributing * ticksPerFrame;

		// How many game ticks will the Animation be really shown (skipped Frames and game ticks removed)
		float relevantAnimationTicksWithSkipping = relevantAnimationTicksForDistribution - (numSkippedFrames * ticksPerFrame);

		if ((flags & AnimationDistributionFlags::ProcessAnimationPending) == AnimationDistributionFlags::ProcessAnimationPending) {
			// If processAnimation will be called after setNewAnimation (in same game tick as setNewAnimation), we increment the Animation-Counter.
			// If no delay is specified, this will result in complete skipped frame (see processAnimation).
			// But if we have a delay specified, this would only result in a reduced time the first frame is shown (one skipped delay).
			// Because of that, we only the remove one game tick from the time the Animation is shown
			relevantAnimationTicksWithSkipping -= 1.F;
			// The Animation Distribution Logic needs to account how many game ticks passed since the Animation started.
			// Because processAnimation will increase this later (in same game tick as setNewAnimation), we correct this upfront.
			// This also means Rendering should never hapen with ticksSinceSequenceStarted_ < 0.
			ticksSinceSequenceStarted_ = -1.F;
		}

		// The preview animation was shown some times (less then one game tick)
		// So we overall have a longer time the animation is shown
		ticksSinceSequenceStarted_ += previewShownGameTickFragments;
		relevantAnimationTicksWithSkipping += previewShownGameTickFragments;

		if ((flags & AnimationDistributionFlags::SkipsDelayOfLastFrame) == AnimationDistributionFlags::SkipsDelayOfLastFrame) {
			// The logic for player/monster/... (not processAnimation) only checks the frame not the delay.
			// That means if a delay is specified, the last-frame is shown less than the other frames
			// Example:
			// If we have a animation with 3 frames and with a delay of 1 (ticksPerFrame = 2).
			// The logic checks "if (frame == 3) { start_new_animation(); }"
			// This will result that frame 4 is the last shown Animation Frame.
			// GameTick		Frame		Cnt
			// 1			1			0
			// 2			1			1
			// 3			2			0
			// 3			2			1
			// 4			3			0
			// 5			-			-
			// in game tick 5 ProcessPlayer sees Frame = 3 and stops the animation.
			// But Frame 3 is only shown 1 game tick and all other Frames are shown 2 game ticks.
			// Thats why we need to remove the Delay of the last Frame from the time (game ticks) the Animation is shown
			relevantAnimationTicksWithSkipping -= (ticksPerFrame - 1);
		}

		// The truncated Frames from previous Animation will also be shown, so we also have to distribute them for the given time (game ticks)
		relevantAnimationTicksForDistribution += (skippedFramesFromPreviousAnimation_ * ticksPerFrame);

		// if we skipped Frames we need to expand the game ticks to make one game tick for this Animation "faster"
		float tickModifier = static_cast<float>(relevantAnimationTicksForDistribution) / relevantAnimationTicksWithSkipping;

		// tickModifier specifies the Animation fraction per game tick, so we have to remove the delay from the variable
		tickModifier /= static_cast<float>(ticksPerFrame);

		relevantFramesForDistributing_ = relevantAnimationFramesForDistributing;
		tickModifier_ = tickModifier;
	}
}

void AnimationInfo::changeAnimationData(OptionalCelSprite celSprite, int8_t numberOfFrames, int8_t ticksPerFrame)
{
	if (numberOfFrames != this->numberOfFrames || ticksPerFrame != this->ticksPerFrame) {
		// Ensure that the currentFrame is still valid and that we disable ADL cause the calculcated values (for example tickModifier_) could be wrong
		if (numberOfFrames >= 1)
			currentFrame = clamp<int8_t>(currentFrame, 0, numberOfFrames - 1);
		else
			currentFrame = -1;

		this->numberOfFrames = numberOfFrames;
		this->ticksPerFrame = ticksPerFrame;
		ticksSinceSequenceStarted_ = 0.F;
		relevantFramesForDistributing_ = 0;
		tickModifier_ = 0.0F;
	}
	this->celSprite = celSprite;
}

void AnimationInfo::processAnimation(bool reverseAnimation /*= false*/, bool dontProgressAnimation /*= false*/)
{
	tickCounterOfCurrentFrame++;
	if (dontProgressAnimation)
		return;
	ticksSinceSequenceStarted_++;
	if (tickCounterOfCurrentFrame >= ticksPerFrame) {
		tickCounterOfCurrentFrame = 0;
		if (reverseAnimation) {
			--currentFrame;
			if (currentFrame == -1) {
				currentFrame = numberOfFrames - 1;
				ticksSinceSequenceStarted_ = 0.F;
			}
		} else {
			++currentFrame;
			if (currentFrame >= numberOfFrames) {
				currentFrame = 0;
				ticksSinceSequenceStarted_ = 0.F;
			}
		}
	}
}

float AnimationInfo::getProgressToNextGameTick() const
{
	if (isPetrified)
		return 0.0F;
	return gfProgressToNextGameTick;
}

} // namespace devilution
