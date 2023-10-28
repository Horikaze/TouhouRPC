import "Touhou08.h";


Touhou08::Touhou08(PROCESSENTRY32W const& pe32) : TouhouBase(pe32) {
    isBoss = 0;
    bossStateChange = 0;
    oldStageFrames = 0;
    spellCardID = 0;
}

Touhou08::~Touhou08() {}

void Touhou08::readDataFromGameProcess() {
    bool oldIsBoss;
    int oldStage; // to reset bossStateChange
    state.gameState = GameState::Playing;
    state.stageState = StageState::Stage;
    state.mainMenuState = MainMenuState::TitleScreen;

    // PLAYER
    character = ReadProcessMemoryInt(processHandle, CHARACTER, 2);
    state.subCharacter = (character <= 3) ? SubCharacter::Team : SubCharacter::Solo;
    switch (character) {
        case 0: state.character = Character::Border; break;
        case 1: state.character = Character::Magic; break;
        case 2: state.character = Character::Scarlet; break;
        case 3: state.character = Character::Nether; break;
        case 4: state.character = Character::Reimu; break;
        case 5: state.character = Character::Yukari; break;
        case 6: state.character = Character::Marisa; break;
        case 7: state.character = Character::Alice; break;
        case 8: state.character = Character::Sakuya; break;
        case 9: state.character = Character::Remilia; break;
        case 10: state.character = Character::Youmu; break;
        case 11: state.character = Character::Yuyuko; break;
    }

    // DIFFICULTY
    difficulty = ReadProcessMemoryInt(processHandle, DIFFICULTY, 1);
    switch (difficulty) {
        default:
        case 0: state.difficulty = Difficulty::Easy; break;
        case 1: state.difficulty = Difficulty::Normal; break;
        case 2: state.difficulty = Difficulty::Hard; break;
        case 3: state.difficulty = Difficulty::Lunatic; break;
        case 4: state.difficulty = Difficulty::Extra; break;
    }

    // FRAMES
    oldStageFrames = stageFrames;
    stageFrames = ReadProcessMemoryInt(processHandle, STAGE_FRAMES);

    // STAGE
    oldStage = stage;
    if (oldStageFrames - stageFrames > 0) {
        oldStage = -1;
    }
    stage = ReadProcessMemoryInt(processHandle, STAGE, 1);
    if (stage != oldStage) {
        bossStateChange = 0;
    }

    // BOSS
    oldIsBoss = isBoss;
    isBoss = ReadProcessMemoryInt(processHandle, BOSS_APPEARANCE, 1);
    if (oldIsBoss != isBoss) {
        bossStateChange++;
    }

    int menuMode = ReadProcessMemoryInt(processHandle, MENU_MODE);
    // menu mode being 2 implies we're in-game

    int stageMode = ReadProcessMemoryInt(processHandle, STAGE_MODE);

    // SPELL_CARD_ID
    if (stageMode & STAGE_MODE_SPELL_PRACTICE_FLAG) {
        spellCardID = ReadProcessMemoryInt(processHandle, SPELLPRAC_CARD_ID, 2);
    }
    else {
        spellCardID = ReadProcessMemoryInt(processHandle, SPELL_CARD_ID);;
    }

    if (menuMode != 2 || (stageMode & STAGE_MODE_DEMO_FLAG) != 0) {
        state.gameState = GameState::MainMenu;

        switch (menuMode) {
            default:
            case 1: state.mainMenuState = MainMenuState::TitleScreen; break;
            case 5: state.mainMenuState = MainMenuState::PlayerData; break;
            case 8: state.mainMenuState = MainMenuState::MusicRoom; break;
        }

        if (state.mainMenuState == MainMenuState::MusicRoom) {
            bgm = ReadProcessMemoryInt(processHandle, MUSIC_ROOM_TRACK);
        }
    }

    if (state.gameState == GameState::Playing) {
        if ((stageMode & STAGE_MODE_SPELL_PRACTICE_FLAG) != 0) {
            state.gameState = GameState::SpellPractice;
        }
        else if ((stageMode & STAGE_MODE_PRACTICE_FLAG) != 0) {
            state.gameState = GameState::StagePractice;
        }
        else if ((stageMode & STAGE_MODE_REPLAY_FLAG) != 0) {
            state.gameState = GameState::WatchingReplay;
        }

        // boss
        if (isBoss) {
            // screw stage 5, 6B, ex
            if (stage == 5 || stage == 7 || stage == 8) {
                switch (bossStateChange) {
                    case 0: // for people who open in middle of a replay
                    case 1: // but really launch this before launching your game
                        state.stageState = StageState::Midboss;
                        break;
                    default:
                        state.stageState = StageState::Boss;
                        break;
                }
            }
            else {
                state.stageState = StageState::Boss;
            }
        }
    }

    // Read current game progress
    int player_pointer = ReadProcessMemoryInt(processHandle, PLAYER_POINTER);
    if (player_pointer) {
        // Behold zungramming: Lives and bombs are actually floats in memory.
        state.lives = static_cast<int>(ReadProcessMemoryFloat(processHandle, (player_pointer + 0x74)));
        state.bombs = static_cast<int>(ReadProcessMemoryFloat(processHandle, (player_pointer + 0x80)));
        state.score = ReadProcessMemoryInt(processHandle, player_pointer);
        state.gameOvers = ReadProcessMemoryInt(processHandle, (player_pointer + 0x28), 1);
    }
}

std::string Touhou08::getStageName() const {
    return stageName[stage];
}

std::string Touhou08::getMidbossName() const {
    return midBossName[stage];
}

std::string Touhou08::getBossName() const {
    return bossName[stage];
}

std::string Touhou08::getSpellCardName() const {
    return th08_spellCardName[spellCardID];
}

std::string Touhou08::getBGMName() const {
    return th08_musicNames[bgm];
}
