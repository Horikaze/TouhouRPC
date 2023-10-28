import "Touhou18.h";

Touhou18::Touhou18(PROCESSENTRY32W const& pe32) : TouhouBase(pe32) {}

void Touhou18::readDataFromGameProcess() {
    char mainMenuState = -1;
    state.gameState = GameState::Playing;
    state.stageState = StageState::Stage;
    gameMode = GAME_MODE_STANDARD;

    // The BGM playing will be used to determine a lot of things
    std::string bgm_playing = ReadProcessMemoryString(processHandle, BGM_STR, 20);

    // Check if the game over music is playing.
    if (bgm_playing == "th128_08.wav") {
        state.gameState = GameState::GameOver;
    }

    // Convert the part after the _ and before the . to int
    // That way it is possible to switch case the BGM playing
    bool prefixBGM = bgm_playing[0] == 'b';
    char bgm_id_str[3]{ bgm_playing[prefixBGM ? 9 : 5], bgm_playing[prefixBGM ? 10 : 6], '\0' };
    bgm = atoi(bgm_id_str);

    difficulty = ReadProcessMemoryInt(processHandle, DIFFICULTY);
    switch (difficulty) {
        default:
        case 0: state.difficulty = Difficulty::Easy; break;
        case 1: state.difficulty = Difficulty::Normal; break;
        case 2: state.difficulty = Difficulty::Hard; break;
        case 3: state.difficulty = Difficulty::Lunatic; break;
        case 4: state.difficulty = Difficulty::Extra; break;
    }

    // Read stage value
    stage = ReadProcessMemoryInt(processHandle, STAGE);

    TouhouAddress menu_pointer = ReadProcessMemoryInt(processHandle, MENU_POINTER);
    if (state.gameState == GameState::Playing && menu_pointer) {
        // The most reliable way of determining our current menu state is through the combination of
        // menu display state and extra flags that get set.
        // This is because of a bug detailed in Touhou14's source file

        /*
            display state (0x18) -> menu screen
            -----------------------------
             0 -> loading
             1 -> main menu
             5 -> game start
             5 -> extra start
             5 -> practice start
            18 -> spell card practice
            12 -> replay
            10 -> player data
            14 -> music room
             3 -> options
            17 -> all manual screens

            ---- sub sub menus ----
             6 -> char select
             7 -> subchar select
             8 -> practice stage select
            19 -> spell card select, N == num spells for stage
            20 -> spell card difficulty select
            12 -> replay stage select
            11 -> player records, N == 9 on shot type, 3 on spell cards
            23 -> achievements
            24 -> ability cards
        */

        int ds = ReadProcessMemoryInt(processHandle, (menu_pointer + 0x18));

        switch (ds) {
            default: state.mainMenuState = MainMenuState::TitleScreen; break;
            case 5:
            case 6:
            case 7:
            {
                // could be normal game, extra, or stage practice, we can check some extra stuff in order to find out.
                if (difficulty == 4) {
                    state.mainMenuState = MainMenuState::ExtraStart;
                }
                else {
                    int practiceFlag = ReadProcessMemoryInt(processHandle, PRACTICE_SELECT_FLAG);
                    state.mainMenuState = (practiceFlag != 0) ? MainMenuState::StagePractice : MainMenuState::GameStart;
                }
                break;
            }
            case 8: state.mainMenuState = MainMenuState::StagePractice; break;
            case 18:
            case 19:
            case 20: state.mainMenuState = MainMenuState::SpellPractice; break;
            case 12: state.mainMenuState = MainMenuState::Replays; break;
            case 10:
            case 11: state.mainMenuState = MainMenuState::PlayerData; break;
            case 23: state.mainMenuState = MainMenuState::Achievements; break;
            case 24: state.mainMenuState = MainMenuState::AbilityCards; break;
            case 14: state.mainMenuState = MainMenuState::MusicRoom; break;
            case 3: state.mainMenuState = MainMenuState::Options; break;
            case 17: state.mainMenuState = MainMenuState::Manual; break;
        }

        mainMenuState = 0;
        state.gameState = GameState::MainMenu;
    }

    if (state.gameState == GameState::Playing) {
        // Note that ZUN's naming for the BGM file names is not very consistent
        switch (bgm) {
            case 0:
            case 1:
                mainMenuState = 0;
                state.mainMenuState = MainMenuState::TitleScreen;
                state.gameState = GameState::MainMenu;
                break;
            case 15: // ending
                state.gameState = GameState::Ending;
                break;
            case 16: // staff roll
                state.gameState = GameState::StaffRoll;
                break;
            default:
                break;
        }
    }

    if (state.gameState != GameState::Playing) {
        // if we're not playing, reset seenMidboss.
        seenMidboss = false;
    }

    if (state.stageState == StageState::Stage) {
        // We can check a stage state number that will confirm if we're in the stage or in the boss

        // Stage state.
        // 0 -> pre-midboss + midboss
        // 2 -> post-midboss
        // 41 -> pre-fight appearance conversations
        // 43 -> boss
        // 81 -> post-boss
        int stageState = ReadProcessMemoryInt(processHandle, STAGE_STATE);
        if (stageState == 0) {
            if (seenMidboss) {
                state.stageState = StageState::Midboss;
            }
            else {
                // If we're in stage state 0, we might be facing a midboss. We can check this to find out:

                // Enemy state object
                // This object holds various information about general ecl state.
                // Offset 210 is some kind of 'boss attack active' flag, so it briefly flicks to 0 between attacks

                // Since it flickers, we can remember that we saw it and just assume we're in a midboss until the stage state or game state changes.
                TouhouAddress enemy_state_ptr = ReadProcessMemoryInt(processHandle, ENEMY_STATE_POINTER);
                if (enemy_state_ptr) {
                    int enemyID = ReadProcessMemoryInt(processHandle, (enemy_state_ptr + 0x210));

                    if (enemyID > 0) {
                        state.stageState = StageState::Midboss;
                        seenMidboss = true;
                    }
                }
            }
        }
        else {
            // reset once we've finished fighting the midboss.
            seenMidboss = false;
            if (stageState == 43) {
                state.stageState = StageState::Boss;
            }
        }
    }

    // Read Spell Card ID (for Spell Practice)
    spellCardID = ReadProcessMemoryInt(processHandle, SPELL_CARD_ID);

    character = ReadProcessMemoryInt(processHandle, CHARACTER);
    switch (character) {
        default:
        case 0: state.character = Character::Reimu; break;
        case 1: state.character = Character::Marisa; break;
        case 2: state.character = Character::Sakuya; break;
        case 3: state.character = Character::Sanae; break;
    }

    // Read current game progress
    state.lives = ReadProcessMemoryInt(processHandle, LIVES);
    state.bombs = ReadProcessMemoryInt(processHandle, BOMBS);
    state.score = ReadProcessMemoryInt(processHandle, SCORE);
    money = ReadProcessMemoryInt(processHandle, MONEY);
    state.gameOvers = ReadProcessMemoryInt(processHandle, GAMEOVERS);

    // Read game mode
    gameMode = static_cast<GameMode>(ReadProcessMemoryInt(processHandle, GAME_MODE));
    switch (gameMode) {
        case GAME_MODE_STANDARD: break; // could be main menu or playing, no need to overwrite anything
        case GAME_MODE_REPLAY: state.gameState = GameState::WatchingReplay; break;
        case GAME_MODE_CLEAR: state.gameState = GameState::StaffRoll; break;
        case GAME_MODE_PRACTICE: state.gameState = GameState::StagePractice; break;
        case GAME_MODE_SPELLPRACTICE: state.gameState = GameState::SpellPractice; break;
    }

    if (state.gameState == GameState::Playing) {
        state.gameState = GameState::Playing_CustomResources;
    }
}

std::string Touhou18::getMidbossName() const {
    switch (stage) {
        case 1: return "Mike Goutokuji";
        case 2: return "Takane Yamashiro";
        case 3: return "Sannyo Komakusa";
        case 4: return "Giant Yin-Yang";
        case 5:
        case 6:
        case 7: return "Tsukasa Kudamaki";
        default: return "";
    }
}

std::string Touhou18::getBossName() const {
    switch (stage) {
        case 1: return "Mike Goutokuji";
        case 2: return "Takane Yamashiro";
        case 3: return "Sannyo Komakusa";
        case 4: return "Misumaru Tamatsukuri";
        case 5: return "Megumu Iizunamaru";
        case 6: return "Chimata Tenkyuu";
        case 7: return "Momoyo Himemushi";
        default: return "";
    }
}

std::string Touhou18::getSpellCardName() const {
    return th18_spellCardName[spellCardID];
}

std::string Touhou18::getBGMName() const {
    return th18_musicNames[bgm];
}

std::string Touhou18::getCustomResources() const {
    std::string resources = std::to_string(state.lives);
    resources.append("/");
    resources.append(std::to_string(state.bombs));
    resources.append("/¥");
    resources.append(std::to_string(money));

    return resources;
}
