#include <vector>

#include "../types.h"
#include "stubs.h"
int menu = 0;
int SelectedItem = 0;
int SelectedItemMax = 0;
int MenuItem[10] = { 5, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

void __cdecl UiDestroy()
{
	DUMMY();
}

BOOL __stdcall UiTitleDialog(int a1)
{
	DUMMY();
	return TRUE;
}

void LoadCharNamesintoMemory(int start, int end)
{
	PkPlayerStruct pkplr;
	int unused;
	void *CharFile;
	char *p_hero_names = *hero_names; // Not sure if this is correct

	memset(hero_names, 0, 0x140u);

	while (start != end) {
		CharFile = pfile_open_save_archive(&unused, start);
		if (CharFile) {
			if (pfile_read_hero(CharFile, &pkplr)) {
				strcpy(p_hero_names, pkplr.pName);
				printf("Player Strength %d\n", (int)pkplr.pBaseStr);
				UnPackPlayer(&pkplr, 0, 0);
				pfile_archive_contains_game(CharFile);
			}
			pfile_SFileCloseArchive(CharFile);
		}

		p_hero_names += 32;
		start++;
	}

	// memcpy(shero_names, hero_names, sizeof(hero_names));
}

void SetMenu(int MenuId)
{
	effects_play_sound("sfx\\items\\titlslct.wav");

	void *tmp = pPcxTitleImage;
	pPcxTitleImage = NULL;
	mem_free_dbg(tmp);
	TitleImageLoaded = false;

	menu = MenuId;
	SelectedItem = 0;
	SelectedItemMax = MenuItem[MenuId] - 1;
}

void ExitDiablo()
{
	printf("Exiting Diablo\n");
	SDL_Quit();
	exit(0);
}

void UiInitialize() // I anticipate to move this later.
{
	SDL_SetRelativeMouseMode(SDL_TRUE);
	// WNDPROC saveProc;

	snd_init(0);
	music_start(5);

	int timestart = 0;

	signed int NewHeroNameIndex = 0;

	SetMenu(0);
	SDL_Event event;
	int x, y;
	bool quit = false;
	int CharsLoaded = 0;
	int HeroPortrait = 3;

	printf("Main Menu Init\n");
	if (!window) {
		SdlDiabloMainWindow();
	}

	ClearScreenBuffer();

	const Uint8 *state = SDL_GetKeyboardState(NULL);

	// static std::deque<MSG> message_queue;

	while (!quit) {
		DrawMouse();
		PaletteFadeIn(32);

		if (menu == 0) {
			SDL_RenderDiabloMainPage();
		}

		if (menu == 2) {
			if (CharsLoaded == 0) {
				LoadCharNamesintoMemory(0, 7);
				//	LoadHeroStats();
				CharsLoaded = 1;
			}
			SDL_RenderDiabloSinglePlayerPage();
			gbMaxPlayers = 1;
			DrawMouse();
			ConstantButtons();
		}

		if (menu == 3) {
			CreateHeroMenu();
			DrawNewHeroKartinka(HeroPortrait, 1);
			ConstantButtons();
			DrawMouse();
		}

		int m4Loaded = 0;
		if (menu == 4) {
			DrawNewHeroKartinka(HeroPortrait, 0);
			RenderDefaultStats(HeroPortrait);
			RenderUndecidedHeroName();
			ConstantButtons();
			DrawMouse();
		}
		if (menu == 5) {
			DrawPreGameOptions(HeroPortrait, 1);
			RenderDefaultStats(HeroPortrait);
			ConstantButtons();
			DrawMouse();
		}
		if (menu == 6) {
			DrawPreGameDifficultySelection(HeroPortrait, 1);
			RenderDefaultStats(HeroPortrait);
			ConstantButtons();
			DrawMouse();
		}

		if (menu == 10) {
			ShowCredts();
		}

		if (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym) {
				case SDLK_ESCAPE:
					if (menu != 0) {
						SetMenu(0);
						break;
					}
					quit = true;
					ExitDiablo();
					break;

				case SDLK_BACKSPACE:
					if (NewHeroNameIndex > 0) {
						HeroUndecidedName[NewHeroNameIndex - 1] = 0;
						--NewHeroNameIndex;
					}
					break;

				case SDLK_UP:
					SelectedItem--;
					if (SelectedItem < 0) {
						SelectedItem = 0;
					}
					effects_play_sound("sfx\\items\\titlemov.wav");
					break;

				case SDLK_DOWN:
					SelectedItem++;
					if (SelectedItem > SelectedItemMax) {
						SelectedItem = SelectedItemMax;
					}
					effects_play_sound("sfx\\items\\titlemov.wav");
					break;

				case SDLK_RETURN:

				default:
					char letter = event.key.keysym.sym;
					if (int(letter) > 96 && int(letter) < 123 || int(letter) == 32)

						if (NewHeroNameIndex < 17) {
							HeroUndecidedName[NewHeroNameIndex] = letter;
							NewHeroNameIndex++;
						}

					break;
				}

			case SDL_KEYUP:
				break;

			default:
				break;
			}

			if (event.type == SDL_QUIT) {
				quit = true;
				ExitDiablo();
			}

			if (event.type == SDL_MOUSEMOTION) {
				// Get the mouse offsets
				MouseX = event.motion.x;
				MouseY = event.motion.y;
			}

			// If a key was pressed
			if (event.type == SDL_MOUSEBUTTONDOWN /*&& event.button.clicks == 2*/) {

				if (event.button.button == SDL_BUTTON_LEFT) {
					x = event.button.x;
					y = event.button.y;
					printf("X %d , Y %d\n", x, y);

					if (menu == 0) {

						int ClickListStart = 230;
						int sizeOfBox = 72;
						int WidthOfBox = 430;
						int ClickListEnd = 343;

						if ((x > ClickListStart) && (y > ClickListStart) && (x < ClickListStart + WidthOfBox) && (y < ClickListStart + sizeOfBox)) { // Single clicked
							printf("SinglePlayer Diablo\n");
							SetMenu(2);
						}

						if ((x > ClickListStart) && (y > ClickListStart + (sizeOfBox)) && (x < ClickListStart + WidthOfBox) && (y < ClickListStart + (sizeOfBox) + sizeOfBox)) { // MultiBox clicked
							printf("MultiPlayer Diablo\n");
						}
						if ((x > ClickListStart) && (y > ClickListStart + (sizeOfBox * 2)) && (x < ClickListStart + WidthOfBox) && (y < ClickListStart + (sizeOfBox * 2) + sizeOfBox)) { // Reply Intro clicked
							printf("Credits\n");
							SetMenu(10);
						}
						if ((x > ClickListStart) && (y > ClickListStart + (sizeOfBox * 4)) && (x < ClickListStart + WidthOfBox) && (y < ClickListStart + (sizeOfBox * 4) + sizeOfBox)) { // ShowCredits clicked
							quit = true;
							ExitDiablo();
						}

					} // End of this Menu0

					if (menu == 2) { // Yes, I know. Skipped 1 and I going to hell for it.
						//	int x = 440;
						//	int y = 430;
						int ClickListStart = 315;
						int sizeOfBox = 30;
						int WidthOfBox = 400;
						int ClickListEnd = 343;

						int CreateHeroY = 555;
						int CreateHeroX = 305;

						SDL_Rect CreateHeroCancelBox;
						CreateHeroCancelBox.y = 550;
						CreateHeroCancelBox.x = 675;
						CreateHeroCancelBox.w = CreateHeroCancelBox.x + 100;
						CreateHeroCancelBox.h = CreateHeroCancelBox.y + 30;

						// Render Clicks
						if (TotalPlayers >= 1 && (x > ClickListStart) && (y > ClickListStart) && (x < ClickListStart + WidthOfBox) && (y < ClickListStart + sizeOfBox)) { // MultiBox clicked
							strcpy(chr_name_str, hero_names[0]);
							printf("Player %s\n", chr_name_str);
							SetMenu(5);
							// break;
						}

						if (TotalPlayers >= 2 && (x > ClickListStart) && (y > ClickListStart + (sizeOfBox)) && (x < ClickListStart + WidthOfBox) && (y < ClickListStart + (sizeOfBox) + sizeOfBox)) { // MultiBox clicked
							printf("Player 2 Diablo\n");
							strcpy(chr_name_str, hero_names[1]);
							printf("Player %s\n", chr_name_str);
							SetMenu(5);
							//	break;
						}

						if (TotalPlayers >= 3 && (x > ClickListStart) && (y > ClickListStart + (sizeOfBox * 2)) && (x < ClickListStart + WidthOfBox) && (y < ClickListStart + (sizeOfBox * 2) + sizeOfBox)) { // MultiBox clicked
							printf("Player 3 Diablo\n");
							strcpy(chr_name_str, hero_names[2]);
							printf("Player %s\n", chr_name_str);
							SetMenu(5);
							//	break;
						}

						if (TotalPlayers >= 4 && (x > ClickListStart) && (y > ClickListStart + (sizeOfBox * 3)) && (x < ClickListStart + WidthOfBox) && (y < ClickListStart + (sizeOfBox * 3) + sizeOfBox)) { // MultiBox clicked
							printf("Player 4 Diablo\n");
							strcpy(chr_name_str, hero_names[3]);
							printf("Player %s\n", chr_name_str);
							SetMenu(5);
							//	break;
						}

						if (TotalPlayers >= 5 && (x > ClickListStart) && (y > ClickListStart + (sizeOfBox * 4)) && (x < ClickListStart + WidthOfBox) && (y < ClickListStart + (sizeOfBox * 4) + sizeOfBox)) { // MultiBox clicked
							printf("Player 5 Diablo\n");
							strcpy(chr_name_str, hero_names[4]);
							printf("Player %s\n", chr_name_str);
							SetMenu(5);
							//	break;
						}

						if (TotalPlayers >= 6 && (x > ClickListStart) && (y > ClickListStart + (sizeOfBox * 5)) && (x < ClickListStart + WidthOfBox) && (y < ClickListStart + (sizeOfBox * 5) + sizeOfBox)) { // MultiBox clicked
							printf("Player 6 Diablo\n");
							strcpy(chr_name_str, hero_names[5]);
							printf("Player %s\n", chr_name_str);
							SetMenu(5);
							//	break;
						}

						if ((x > CreateHeroCancelBox.x) && (y > CreateHeroCancelBox.y) && (x < CreateHeroCancelBox.w) && (y < CreateHeroCancelBox.h)) {
							HeroPortrait = 3;

							printf("Cancel\n\n\n");
							SetMenu(0);
						}
						if ((x > CreateHeroX) && (y > CreateHeroY) && (x < CreateHeroX + WidthOfBox) && (y < CreateHeroY + sizeOfBox)) {
							printf("Clicked Create Hero Box\n");
							SetMenu(3);
						}
					}
					if (menu == 3) {
						// SinglePlayerMenuItemsLoaded = 0;
						printf("\n\nmenu3 X%d Y%d \n ", x, y);

						int WarriorSelectBoxY = 430;
						int WarriorSelectBoxW = 100;
						int WarriorSelectBoxS = 100;

						int RogueSelectBoxX = 450;
						int RogueSelectBoxY = 392;
						int SorcerorSelectBoxX = 383;
						int SorcerorSelectBoxY = 365;
						int CreateHeroOkBoxX = 330;
						int CreateHeroOkBoxY = 441;
						int CreateHeroCanBBoxX = 445;
						int CreateHeroCanBBoxY = 473;

						// int x = 280;
						// int y = 430;

						SDL_Rect WarriorSelectBox;
						WarriorSelectBox.y = 350;
						WarriorSelectBox.x = 280;
						WarriorSelectBox.w = WarriorSelectBox.x + 100;
						WarriorSelectBox.h = WarriorSelectBox.y + 30;

						SDL_Rect RogueSelectBox;
						RogueSelectBox.y = 392;
						RogueSelectBox.x = 280;
						RogueSelectBox.w = RogueSelectBox.x + 100;
						RogueSelectBox.h = RogueSelectBox.y + 30;
						// X450 Y 392 ;

						SDL_Rect SorcerorSelectBox;
						SorcerorSelectBox.y = 428;
						SorcerorSelectBox.x = 280;
						SorcerorSelectBox.w = SorcerorSelectBox.x + 100;
						SorcerorSelectBox.h = SorcerorSelectBox.y + 30;
						// X 447 Y 428

						SDL_Rect CreateHeroCancelBox;
						CreateHeroCancelBox.y = 550;
						CreateHeroCancelBox.x = 675;
						CreateHeroCancelBox.w = CreateHeroCancelBox.x + 100;
						CreateHeroCancelBox.h = CreateHeroCancelBox.y + 30;

						if ((x > WarriorSelectBox.x) && (y > WarriorSelectBox.y) && (x < WarriorSelectBox.w) && (y < WarriorSelectBox.h)) {

							printf(" warrior I was hit\n\n\n");
							HeroPortrait = 0;
							HeroChosen = 0;
							SetMenu(4);
						}
						if ((x > RogueSelectBox.x) && (y > RogueSelectBox.y) && (x < RogueSelectBox.w) && (y < RogueSelectBox.h)) {

							printf(" rogue I was hit\n\n\n");
							HeroPortrait = 1;
							HeroChosen = 1;
							SetMenu(4);
						}
						if ((x > SorcerorSelectBox.x) && (y > SorcerorSelectBox.y) && (x < SorcerorSelectBox.w) && (y < SorcerorSelectBox.h)) {
							HeroPortrait = 2;
							printf("sorceror I was hit\n\n\n");
							HeroChosen = 2;
							SetMenu(4);
						}

						if ((x > CreateHeroCancelBox.x) && (y > CreateHeroCancelBox.y) && (x < CreateHeroCancelBox.w) && (y < CreateHeroCancelBox.h)) {
							HeroPortrait = 3;

							printf("Cancel\n\n\n");
							--menu;
						}
					}
					if (menu == 4) {
						printf("sozdat geroya");
						SDL_Rect CreateHeroCancelBox;
						CreateHeroCancelBox.y = 550;
						CreateHeroCancelBox.x = 675;
						CreateHeroCancelBox.w = CreateHeroCancelBox.x + 100;
						CreateHeroCancelBox.h = CreateHeroCancelBox.y + 30;

						// X 549 , Y 551
						SDL_Rect ClickOkBox;
						ClickOkBox.y = 550;
						ClickOkBox.x = 550;
						ClickOkBox.w = ClickOkBox.x + 30;
						ClickOkBox.h = ClickOkBox.y + 30;

						if ((x > CreateHeroCancelBox.x) && (y > CreateHeroCancelBox.y) && (x < CreateHeroCancelBox.w) && (y < CreateHeroCancelBox.h)) {
							memset(HeroUndecidedName, 0, 17);

							NewHeroNameIndex = 0;

							printf("Cancel\n\n\n");
							HeroPortrait = 3;
							SetMenu(3);
						}

						if ((x > ClickOkBox.x) && (y > ClickOkBox.y) && (x < ClickOkBox.w) && (y < ClickOkBox.h)) {

							printf("Ok\n");
							CreateSinglePlayerChar = 1;
							const char *test_name = HeroUndecidedName;
							printf("%s\n", test_name);
							break;
						}
					}

					clock_t start, end;
					double cpu_time_used;

					if (menu == 5) {
						if (timestart == 0) {
							start = clock();
							timestart = 1;
						}

						end = clock();

						cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;

						printf("TIEM DELAY %f\n", cpu_time_used);

						int CreateHeroOkBoxX = 330;
						int CreateHeroOkBoxY = 441;
						int CreateHeroCanBBoxX = 445;
						int CreateHeroCanBBoxY = 473;

						SDL_Rect NewGameBox;
						NewGameBox.y = 350;
						NewGameBox.x = 280;
						NewGameBox.w = NewGameBox.x + 300;
						NewGameBox.h = NewGameBox.y + 30;

						SDL_Rect LoadGameBox;
						LoadGameBox.y = 392;
						LoadGameBox.x = 280;
						LoadGameBox.w = LoadGameBox.x + 300;
						LoadGameBox.h = LoadGameBox.y + 30;
						// X450 Y 392 ;

						SDL_Rect SorcerorSelectBox;
						SorcerorSelectBox.y = 428;
						SorcerorSelectBox.x = 280;
						SorcerorSelectBox.w = SorcerorSelectBox.x + 100;
						SorcerorSelectBox.h = SorcerorSelectBox.y + 30;
						// X 447 Y 428

						SDL_Rect CreateHeroCancelBox;
						CreateHeroCancelBox.y = 550;
						CreateHeroCancelBox.x = 675;
						CreateHeroCancelBox.w = CreateHeroCancelBox.x + 100;
						CreateHeroCancelBox.h = CreateHeroCancelBox.y + 30;

						if (cpu_time_used > 0.5 && (x > NewGameBox.x) && (y > NewGameBox.y) && (x < NewGameBox.w) && (y < NewGameBox.h)) {

							printf(" New Game I was hit\n\n\n");
							SetMenu(6);
							cpu_time_used = 0;
							timestart = 0;
							start = 0;
						}
						if (cpu_time_used > 0.5 && (x > LoadGameBox.x) && (y > LoadGameBox.y) && (x < LoadGameBox.w) && (y < LoadGameBox.h)) {

							printf(" Load Game I was hit\n\n\n");
							break;
						}

						if ((x > CreateHeroCancelBox.x) && (y > CreateHeroCancelBox.y) && (x < CreateHeroCancelBox.w) && (y < CreateHeroCancelBox.h)) {
							HeroPortrait = 3;
							timestart = 0;
							cpu_time_used = 0;
							start = 0;
							end = 0;
							cpu_time_used = 0;

							printf("Cancel\n\n\n");

							SetMenu(2); // Return back to select hero menu.
						}
					}

					if (menu == 6) {
						// Choose difficulty

						if (timestart == 0) {
							start = clock();
							timestart = 1;
						}

						end = clock();
						cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;

						int CreateHeroOkBoxX = 330;
						int CreateHeroOkBoxY = 441;
						int CreateHeroCanBBoxX = 445;
						int CreateHeroCanBBoxY = 473;

						// int x = 280;
						// int y = 430;

						SDL_Rect NormalSelectBox;
						NormalSelectBox.y = 350;
						NormalSelectBox.x = 280;
						NormalSelectBox.w = NormalSelectBox.x + 300;
						NormalSelectBox.h = NormalSelectBox.y + 30;

						SDL_Rect NightmareSelectBox;
						NightmareSelectBox.y = 392;
						NightmareSelectBox.x = 280;
						NightmareSelectBox.w = NightmareSelectBox.x + 300;
						NightmareSelectBox.h = NightmareSelectBox.y + 30;
						// X450 Y 392 ;

						SDL_Rect HellSelectBox;
						HellSelectBox.y = 428;
						HellSelectBox.x = 280;
						HellSelectBox.w = HellSelectBox.x + 300;
						HellSelectBox.h = HellSelectBox.y + 30;
						// X 447 Y 428

						SDL_Rect CreateHeroCancelBox;
						CreateHeroCancelBox.y = 550;
						CreateHeroCancelBox.x = 675;
						CreateHeroCancelBox.w = CreateHeroCancelBox.x + 100;
						CreateHeroCancelBox.h = CreateHeroCancelBox.y + 30;

						if (cpu_time_used > 0.5 && (x > NormalSelectBox.x) && (y > NormalSelectBox.y) && (x < NormalSelectBox.w) && (y < NormalSelectBox.h)) {
							StartNewGame = 1;
							gnDifficulty = DIFF_NORMAL;
							break;
						}
						if (cpu_time_used > 0.5 && (x > NightmareSelectBox.x) && (y > NightmareSelectBox.y) && (x < NightmareSelectBox.w) && (y < NightmareSelectBox.h)) {
							StartNewGame = 1;
							gnDifficulty = DIFF_NIGHTMARE;
							break;
						}
						if (cpu_time_used > 1 && (x > HellSelectBox.x) && (y > HellSelectBox.y) && (x < HellSelectBox.w) && (y < HellSelectBox.h)) {
							gnDifficulty = DIFF_HELL;
							StartNewGame = 1;
							break;
						}

						if ((x > CreateHeroCancelBox.x) && (y > CreateHeroCancelBox.y) && (x < CreateHeroCancelBox.w) && (y < CreateHeroCancelBox.h)) {
							HeroPortrait = 3;
							timestart = 0;
							cpu_time_used = 0;
							start = 0;
							end = 0;
							cpu_time_used = 0;

							printf("Cancel\n\n\n");
							--menu;
						}
					}
				}
			}
		}
	}
}

static std::vector<_uiheroinfo> hero_infos;

static BOOL __stdcall ui_add_hero_info(_uiheroinfo *info)
{
	hero_infos.emplace_back(*info);
	return TRUE;
}

BOOL __stdcall UiSelHeroSingDialog(BOOL(__stdcall *fninfo)(BOOL(__stdcall *fninfofunc)(_uiheroinfo *)),
                                   BOOL(__stdcall *fncreate)(_uiheroinfo *), BOOL(__stdcall *fnremove)(_uiheroinfo *),
                                   BOOL(__stdcall *fnstats)(int, _uidefaultstats *), int *dlgresult, char *name,
                                   int *difficulty)
{
	DUMMY();

	hero_infos.clear();
	fninfo(&ui_add_hero_info);

	if (CreateSinglePlayerChar) {
		const char *test_name = HeroUndecidedName;
		DUMMY_PRINT("create hero: %s", test_name);

		strcpy(name, test_name);

		_uiheroinfo hero_info = {1};
		strcpy(hero_info.name, test_name);
		hero_info.heroclass = HeroChosen;

		fncreate(&hero_info);
	}

	// If a hero is available, load it, otherwise create a new one
	// if (!hero_infos.empty()) {

	else {
				// Yes, I undestand that this means new players can start a hell game.
				// I like this.

		*difficulty = gnDifficulty ;


		if (StartNewGame) {
			const char *hero_name = chr_name_str;
			DUMMY_PRINT("New Game use hero: %s\n", hero_name);
			strcpy(name, hero_name);
			printf("Difficulty : %d \n",* difficulty);


		} else {
			const char *hero_name = chr_name_str;
			DUMMY_PRINT("Loading Game : use hero: %s\n", hero_name);
			strcpy(name, hero_name);

			*dlgresult = 2; // This means load game
			printf("Difficulty : %d \n",* difficulty);

		}
	}

	return TRUE;
}

BOOL __stdcall UiMainMenuDialog(char *name, int *pdwResult, void(__stdcall *fnSound)(char *file), int a4)
{
	DUMMY();
	// Pretend we selected Single Player from the main menu
	*pdwResult = MAINMENU_SINGLE_PLAYER;
	return TRUE;
}

int __cdecl UiProfileGetString()
{
	DUMMY();
	return 0;
}

void __stdcall UiSetupPlayerInfo(char *infostr, _uiheroinfo *pInfo, int type)
{
	DUMMY_PRINT("chr: %s", infostr);
}

BOOL __stdcall UiCopyProtError(int *pdwResult)
{
	UNIMPLEMENTED();
}

void __stdcall UiAppActivate(BOOL bActive)
{
	UNIMPLEMENTED();
}

BOOL __fastcall UiValidPlayerName(char *name)
{
	UNIMPLEMENTED();
}

int __stdcall UiProgressDialog(HWND window, char *msg, int a3, void *fnfunc, int a5)
{
	UNIMPLEMENTED();
}

BOOL __stdcall UiSelHeroMultDialog(BOOL(__stdcall *fninfo)(BOOL(__stdcall *fninfofunc)(_uiheroinfo *)),
                                   BOOL(__stdcall *fncreate)(_uiheroinfo *), BOOL(__stdcall *fnremove)(_uiheroinfo *),
                                   BOOL(__stdcall *fnstats)(int, _uidefaultstats *), int *dlgresult, int *a6,
                                   char *name)
{
	UNIMPLEMENTED();
}

BOOL __stdcall UiCreditsDialog(int a1)
{
	UNIMPLEMENTED();
}

void __cdecl UiProfileCallback()
{
	UNIMPLEMENTED();
}

void __cdecl UiProfileDraw()
{
	UNIMPLEMENTED();
}

BOOL __stdcall UiCategoryCallback(int a1, int a2, int a3, int a4, int a5, _DWORD *a6, _DWORD *a7)
{
	UNIMPLEMENTED();
}

BOOL __stdcall UiGetDataCallback(int game_type, int data_code, void *a3, int a4, int a5)
{
	UNIMPLEMENTED();
}

BOOL __stdcall UiAuthCallback(int a1, char *a2, char *a3, char a4, char *a5, LPSTR lpBuffer, int cchBufferMax)
{
	UNIMPLEMENTED();
}

BOOL __stdcall UiSoundCallback(int a1, int type, int a3)
{
	UNIMPLEMENTED();
}

void __stdcall UiMessageBoxCallback(HWND hWnd, char *lpText, LPCSTR lpCaption, UINT uType)
{
	UNIMPLEMENTED();
}

BOOL __stdcall UiDrawDescCallback(int arg0, COLORREF color, LPCSTR lpString, char *a4, int a5, UINT align, time_t a7,
                                  HDC *a8)
{
	UNIMPLEMENTED();
}

BOOL __stdcall UiCreateGameCallback(int a1, int a2, int a3, int a4, int a5, int a6)
{
	UNIMPLEMENTED();
}

BOOL __stdcall UiArtCallback(int game_type, unsigned int art_code, PALETTEENTRY *pPalette, void *pBuffer,
                             DWORD dwBuffersize, DWORD *pdwWidth, DWORD *pdwHeight, DWORD *pdwBpp)
{
	UNIMPLEMENTED();
}

int __stdcall UiSelectGame(int a1, _SNETPROGRAMDATA *client_info, _SNETPLAYERDATA *user_info, _SNETUIDATA *ui_info,
                           _SNETVERSIONDATA *file_info, int *a6)
{
	UNIMPLEMENTED();
}

int __stdcall UiSelectProvider(int a1, _SNETPROGRAMDATA *client_info, _SNETPLAYERDATA *user_info, _SNETUIDATA *ui_info,
                               _SNETVERSIONDATA *file_info, int *type)
{
	UNIMPLEMENTED();
}

int __stdcall UiCreatePlayerDescription(_uiheroinfo *info, int mode, char *desc)
{
	UNIMPLEMENTED();
}
