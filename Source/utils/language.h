#pragma once

#include <string>

#include "utils/stdcompat/string_view.hpp"

#define _(x) LanguageTranslate(x)
#define ngettext(x, y, z) LanguagePluralTranslate(x, y, z)
#define pgettext(context, x) LanguageParticularTranslate(context, x)
#define N_(x) (x)
#define P_(context, x) (x)

bool HasTranslation(const std::string &locale);
void LanguageInitialize();

/**
 * @brief Returns the translation for the given key.
 *
 * @return guaranteed to be null-terminated if `key` is.
 */
devilution::string_view LanguageTranslate(devilution::string_view key);

/**
 * @brief Returns a singular or plural translation for the given keys and count.
 *
 * @return guaranteed to be null-terminated if `key` is.
 */
devilution::string_view LanguagePluralTranslate(devilution::string_view singular, devilution::string_view plural, int count);

/**
 * @brief Returns the translation for the given key and context identifier.
 *
 * @return guaranteed to be null-terminated if `key` is.
 */
devilution::string_view LanguageParticularTranslate(devilution::string_view context, devilution::string_view message);

// Chinese and Japanese, and Korean small font is 16px instead of a 12px one for readability.
bool IsSmallFontTall();
