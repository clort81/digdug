#include "../include/Game.h"

Game::Game(){

    isRunning = true;
    srand(time(NULL));
    redraw = false;
    actualPressedKey = ALLEGRO_KEY_SPACE;
    al_init();
    createDisplay();
    al_init_image_addon();
    al_install_keyboard();
    al_init_font_addon();   
    al_init_ttf_addon();
    al_init_acodec_addon();
    al_install_audio();
    initGameMaps();
    loadLevel(1);
    loadBestScore();
    loadAudios();
    initGameOjects();
    al_rest(1.0);
    mainTimer = al_create_timer(1.0 / FPS);
    swallowTimer = al_create_timer(3);
    bonusTimer = al_create_timer(60);
    al_start_timer(mainTimer);
    eventQueue = al_create_event_queue();
    al_register_event_source(eventQueue, al_get_display_event_source(mainDisplay));
    al_register_event_source(eventQueue, al_get_keyboard_event_source());
    al_register_event_source(eventQueue, al_get_timer_event_source(mainTimer));
    al_register_event_source(eventQueue, al_get_timer_event_source(swallowTimer));
    al_register_event_source(eventQueue, al_get_timer_event_source(bonusTimer));
    centerGround = al_load_bitmap("res/images/ground/ground2.png");
    middleGround = al_load_bitmap("res/images/ground/ground1.png");
    cornerGround = al_load_bitmap("res/images/ground/ground3.png");
    background = al_load_bitmap("res/images/background.png");
}

void Game::createDisplay(){
    al_set_new_display_flags(ALLEGRO_FULLSCREEN_WINDOW);
    al_get_monitor_info(0, &monitorInfos);
    mainDisplay = al_create_display(monitorInfos.x2, monitorInfos.y2);
    buffer = al_create_bitmap(nativeScreenHeight, nativeScreenHeight);
    float scaleW = static_cast<float>(monitorInfos.x2) / static_cast<float>(nativeScreenWidth);
    float scaleH = static_cast<float>(monitorInfos.y2) / static_cast<float>(nativeScreenHeight);
    scale = min(scaleW, scaleH);
   
}

void Game::createMenu(){
    if(!showControls)
        al_draw_bitmap(al_load_bitmap("res/images/menu/menu.png") ,0,0,0);
    else
        al_draw_bitmap(al_load_bitmap("res/images/menu/menu2.png") ,0,0,0);
}

void Game::loadBestScore(){
    ifstream bScoreFile;
    bScoreFile.open("score/best_score.txt", ios::in);
    bScoreFile >> bestScore;
    if(bestScore < 0)
        bestScore = 0;
    
    bScoreFile.close();
}

void Game::initGameMaps(){

    for(int i = 0; i < originalCollisionMapHeight * collisionCellDivider; i++){
        for(int j = 0; j < originalCollisionMapWidth * collisionCellDivider; j++){
            collisionMap[i][j] = 0;
        }
    }

    for(int i = 0; i < originalCollisionMapHeight * collisionCellDivider; i++){
        for(int j = 0; j < originalCollisionMapWidth * collisionCellDivider; j++){
            groundMap[i][j] = 0;
        }
    }

    for(int i = 0; i < initialDiggedGroundIndex; i++){
        for(int j = 0; j < originalCollisionMapWidth * collisionCellDivider; j++){
            groundMap[i][j] = 1;
        }
    }
}

void::Game::quit(){
    isRunning = false;
    al_destroy_display(mainDisplay);
}
void Game::checkendGame(){

    if(actualPressedKey == ALLEGRO_KEY_ESCAPE)
        quit();

}

void Game::checkStopGame(){
    Text pauseText(40,al_map_rgb(255,255,255), nativeScreenWidth / 2, (nativeScreenHeight / 2) - 20, "PAUSE");
    pauseText.drawOnScreen();
    al_stop_sample(&ret);
    isGameInPause = true;

}

void Game::eventManager(){

    al_wait_for_event(eventQueue, &actualEvent);

    if(actualEvent.type == ALLEGRO_EVENT_TIMER){
        if(actualEvent.any.source == al_get_timer_event_source(mainTimer)){
            redraw = true;
            al_get_keyboard_state(&keyboardState);

            if(!al_key_down(&keyboardState, actualPressedKey))
                actualPressedKey = ALLEGRO_KEY_SPACE;

            if(swallowValue >= groundIndexStart){
                isSwallowTimerActive = false;
                al_stop_timer(swallowTimer);
                swallowValue = 0;
            }
        } else if(actualEvent.any.source == al_get_timer_event_source(swallowTimer)) {
            isSwallowTimerActive = false;
            al_stop_timer(swallowTimer);
            swallowValue = 0;
        } else if(actualEvent.any.source == al_get_timer_event_source(bonusTimer)) {
            gameObjs.push_back(new Powerups(104, 132, 16, 16));
        }

    } else if(actualEvent.type == ALLEGRO_EVENT_KEY_DOWN){
        switch (actualEvent.keyboard.keycode)
        {
            case ALLEGRO_KEY_ESCAPE:
                actualPressedKey = ALLEGRO_KEY_ESCAPE;
                break;
            case ALLEGRO_KEY_UP:
                actualPressedKey = ALLEGRO_KEY_UP;
                break;
            case ALLEGRO_KEY_LEFT:
                actualPressedKey = ALLEGRO_KEY_LEFT;
                break;
            case ALLEGRO_KEY_RIGHT:
                actualPressedKey = ALLEGRO_KEY_RIGHT;
                break;
            case ALLEGRO_KEY_DOWN:
                actualPressedKey = ALLEGRO_KEY_DOWN;
                break;
            case ALLEGRO_KEY_D:
                actualPressedKey = ALLEGRO_KEY_D;
                break;
            case ALLEGRO_KEY_V:
                if(!setPause){
                    setPause = true;
                } else {
                    isGameInPause = false;
                    setPause = false;
                    al_play_sample(audios[BACKGROUND_SOUND], 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_LOOP, &ret);
                }
                break;
            case ALLEGRO_KEY_ENTER:
                if(!showControls){
                    showControls = true;
                } else {
                    showMenu = false;
                    al_play_sample(audios[BACKGROUND_SOUND], 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_LOOP, &ret);
                    al_start_timer(bonusTimer);
                }
                break;
                    
            default:
                break;
        }
    }
}

bool Game::isGameRunning(){
    return isRunning;
}

void Game::loadLevel(int difficulty){
    actualLevel = new Level(difficulty);
    gameObjs = actualLevel->getGameObjects();
    enemiesCounter = actualLevel->getEnemiesNumber();
}

void Game::initGameOjects(){

    gameObjs.push_back(new LifePoints(0,274,16,16));
    gameObjs.push_back(new Text(10, al_map_rgb(255,0,0), 110, 0, "HIGH SCORE"));
    gameObjs.push_back(new Text(10, al_map_rgb(255,255,255), 110, 12,bestScore));
    gameObjs.push_back(new Text(10, al_map_rgb(255,255,255), 100, 274,"SCORE"));
    gameObjs.push_back(new Score(10, al_map_rgb(255,255,255), 142, 274));
    gameObjs.push_back(new Text(10, al_map_rgb(255,255,255), 200, 274, "ROUND "));
    gameObjs.push_back(new Text(10, al_map_rgb(255,255,255), 220, 274, round));
    if(!showMenu)
        al_play_sample(audios[BACKGROUND_SOUND], 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_LOOP, &ret);

}

void Game::loadAudios(){
    al_reserve_samples(10);
    audios.push_back(al_load_sample("res/audio/03-DigDug Walking.wav"));
    audios.push_back(al_load_sample("res/audio/22-Harpoon.wav"));
    audios.push_back(al_load_sample("res/audio/23-Pumping.wav"));
    audios.push_back(al_load_sample("res/audio/09-Monster touched DigDug.wav"));
    audios.push_back(al_load_sample("res/audio/10-DidDug Disappearing.wav"));
    audios.push_back(al_load_sample("res/audio/17-Monster Squashed.wav"));
    audios.push_back(al_load_sample("res/audio/11-Game Over Music.wav"));
    audios.push_back(al_load_sample("res/audio/08-DigDug Walking.wav"));
    audios.push_back(al_load_sample("res/audio/05-Round Clear Music.wav"));
}

void Game::updateGround(){
    for(int i = initialDiggedGroundIndex; i < originalCollisionMapHeight * collisionCellDivider; i++){
        for(int j = 1; j < originalCollisionMapWidth * collisionCellDivider - 1; j++){
            if(groundMap[i][j] == 1 && (groundMap[i + 1][j] == 0 || groundMap[i + 1][j] == STONE)){
                if(groundMap[i][j + 1] == 1 && groundMap[i][j - 1] == 1)
                    al_draw_bitmap(middleGround, j * collisionCellDivider, i * collisionCellDivider, 0);
                else if(groundMap[i][j + 1] == 0 || groundMap[i][j + 1] == STONE)
                    al_draw_bitmap(cornerGround, j * collisionCellDivider, i * collisionCellDivider, 0);
                else if(groundMap[i][j - 1] == 0 || groundMap[i][j - 1] == STONE)
                    al_draw_bitmap(cornerGround, j * collisionCellDivider, i * collisionCellDivider, ALLEGRO_FLIP_HORIZONTAL);
            } else if(groundMap[i][j] == 1 && (groundMap[i - 1][j] == 0 || groundMap[i - 1][j] == STONE)){
                if(groundMap[i][j + 1] == 1 && groundMap[i][j - 1] == 1)
                    al_draw_bitmap(middleGround, j * collisionCellDivider, i * collisionCellDivider, ALLEGRO_FLIP_VERTICAL);
                else if(groundMap[i][j + 1] == 0 || groundMap[i][j + 1] == STONE)
                    al_draw_bitmap(cornerGround, j * collisionCellDivider, i * collisionCellDivider, ALLEGRO_FLIP_VERTICAL);
                else if(groundMap[i][j - 1] == 0 || groundMap[i][j - 1] == STONE)
                    al_draw_bitmap(cornerGround, j * collisionCellDivider, i * collisionCellDivider, 3);
            } else if(groundMap[i][j] == 1 && groundMap[i + 1][j] == 1 && groundMap[i - 1][j] == 1){
                if(groundMap[i][j - 1] == 0 || groundMap[i][j - 1] == STONE)
                    al_draw_rotated_bitmap(middleGround, (groundSpiteSize / 2), (groundSpiteSize / 2), j * collisionCellDivider + (groundSpiteSize / 2), i * collisionCellDivider + (groundSpiteSize / 2), radiantFor90Degrees, 0);
                else if(groundMap[i][j + 1] == 0 || groundMap[i][j + 1] == STONE)
                    al_draw_rotated_bitmap(middleGround, (groundSpiteSize / 2), (groundSpiteSize / 2), j * collisionCellDivider + (groundSpiteSize / 2), i * collisionCellDivider + (groundSpiteSize / 2), radiantFor270Degrees, 0);
                else if(groundMap[i][j + 1] == 1 && groundMap[i][j - 1] == 1)
                    al_draw_bitmap(centerGround, j * collisionCellDivider, i * collisionCellDivider, 0);
            }
        }
    }

    for(int j = 0; j < originalCollisionMapWidth * collisionCellDivider; j++){
        if(groundMap[initialDiggedGroundIndex][j] == 1){
            al_draw_bitmap(centerGround, j * collisionCellDivider, (initialDiggedGroundIndex - 1) * collisionCellDivider + (groundSpiteSize / 2), 0);
            al_draw_bitmap(centerGround, j * collisionCellDivider, initialDiggedGroundIndex * collisionCellDivider, 0);
        }
    }

    for(int i = initialDiggedGroundIndex; i < originalCollisionMapHeight * collisionCellDivider; i++){
        if(groundMap[i][0] == 1){
            if(groundMap[i + 1][0] == 1){
                al_draw_rotated_bitmap(middleGround, groundSpiteSize / 2, groundSpiteSize / 2, 0 + (groundSpiteSize / 2), i * collisionCellDivider + (groundSpiteSize / 2), radiantFor90Degrees, 0);
            } else if(groundMap[i + 1][0] == 0){
                al_draw_bitmap(cornerGround, 0, i * collisionCellDivider, ALLEGRO_FLIP_HORIZONTAL);
            }
        }
    }

    for(int i = initialDiggedGroundIndex; i < originalCollisionMapHeight * collisionCellDivider; i++){
        if(groundMap[i][originalCollisionMapWidth * collisionCellDivider - 1] == 1){
            if(groundMap[i + 1][originalCollisionMapWidth * collisionCellDivider - 1] == 1){
                al_draw_rotated_bitmap(middleGround, (groundSpiteSize / 2), (groundSpiteSize / 2), (originalCollisionMapWidth * collisionCellDivider - 1) * collisionCellDivider + (groundSpiteSize / 2), i * collisionCellDivider + (groundSpiteSize / 2), radiantFor270Degrees, 0);
            } else if(groundMap[i + 1][originalCollisionMapWidth * collisionCellDivider - 1] == 0){
                al_draw_bitmap(cornerGround, (originalCollisionMapWidth * collisionCellDivider - 1) * collisionCellDivider, i * collisionCellDivider, 0);
            }
        }
    }
}

void Game::drawScene(){

    if(redraw && al_is_event_queue_empty(eventQueue) && !isGameInPause){
        al_set_target_bitmap(buffer);
        al_clear_to_color(al_map_rgb(0, 0, 0));
        if(showMenu){
            createMenu();
        } else {
            al_draw_bitmap(background, 0, 0, 0);
            updateGround();

            for(int i = 0; i < gameObjs.size(); i++){
                if(gameObjs[i]->getVisible())
                    gameObjs[i]->drawOnScreen();                
            }

            if(setPause)
                checkStopGame();
        }
        al_set_target_backbuffer(mainDisplay);
        al_clear_to_color(al_map_rgb(0, 0, 0));
        al_draw_scaled_bitmap(buffer, 0, 0, nativeScreenWidth, nativeScreenHeight, (monitorInfos.x2 - (nativeScreenWidth * scale)) / 2, (monitorInfos.y2 - (nativeScreenHeight * scale)) / 2, nativeScreenWidth * scale, nativeScreenHeight * scale, 0);
        al_flip_display();
        redraw = false;

        if(enemiesCounter == 0 || !gameObjs[0]->getVisible()){
            resetGameScene();
        } else if(enemiesCounter == 1 && !lastEnemySong) {
            al_stop_sample(&ret);
            ALLEGRO_SAMPLE* temp = audios[BACKGROUND_SOUND];
            audios[BACKGROUND_SOUND] = audios[LAST_ENEMY];
            audios[LAST_ENEMY] = temp;
            al_play_sample(audios[BACKGROUND_SOUND], 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_LOOP, &ret);
            lastEnemySong = true;
        }

    }

}

void Game::drawFinalScene(){
    al_set_target_bitmap(buffer);
    al_clear_to_color(al_map_rgb(0, 0, 0));
    al_draw_bitmap(background, 0, 0, 0);
    updateGround();
    Text gameOver(20, al_map_rgb(255,255,255), nativeScreenWidth / 2, nativeScreenHeight / 2, "GAME OVER");
    gameOver.drawOnScreen();
    al_set_target_backbuffer(mainDisplay);
    al_clear_to_color(al_map_rgb(0, 0, 0));
    al_draw_scaled_bitmap(buffer, 0, 0, nativeScreenWidth, nativeScreenHeight, (monitorInfos.x2 - (nativeScreenWidth * scale)) / 2, (monitorInfos.y2 - (nativeScreenHeight * scale)) / 2, nativeScreenWidth * scale, nativeScreenHeight * scale, 0);
    al_flip_display();
}

void Game::resetGameScene(){

    if(lifePoints > 0 && enemiesCounter == 0){
        al_rest(1.0);
        al_stop_sample(&ret);
        al_play_sample(audios[ROUND_WIN], 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, &ret);
        al_rest(3.0);
        deleteGameObjects();
        initGameMaps();
        round++;
        loadLevel(round);
        ALLEGRO_SAMPLE* temp = audios[LAST_ENEMY];
        audios[LAST_ENEMY] = audios[BACKGROUND_SOUND];
        audios[BACKGROUND_SOUND] = temp;
        lastEnemySong = false;
        initGameOjects();
    } else if(lifePoints > 0 && enemiesCounter != 0) {
        al_rest(1.0);
        al_stop_sample(&ret);
        resetGameObjects();
        gameObjs[0]->setVisible();
        died = false;
    } else if(lifePoints == 0){
        isBackgroundAudioOn = false;
        al_stop_sample(&ret);
        al_play_sample(audios[GAME_OVER], 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, &ret);
        drawFinalScene();
        saveBestScore();
        al_rest(2.0);
        quit();
    }
    
}

void Game::updateGameScene(){
    
    drawScene();
    eventManager();
    checkendGame();

}

void Game::resetGameObjects(){

    for(int i = 0; i < originalCollisionMapHeight * collisionCellDivider; i++){
        for(int j = 0; j < originalCollisionMapWidth * collisionCellDivider; j++){
            collisionMap[i][j] = 0;
        }
    }


    for(int i = 0; i < gameObjs.size(); i++){
        gameObjs[i]->restoreCoordinates();
    }

    al_play_sample(audios[BACKGROUND_SOUND], 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_LOOP, &ret);
}

void Game::deleteGameObjects(){

    for(int i = 0; i < gameObjs.size(); i++){
        delete gameObjs[i];
    }

    delete actualLevel;
}

void Game::saveBestScore(){
    if(score > bestScore){
        ofstream bScoreFile;
        bScoreFile.open("score/best_score.txt", ios::trunc);
        bScoreFile << score;
    }
}


Game::~Game(){
    deleteGameObjects();
}
