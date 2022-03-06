// Fill out your copyright notice in the Description page of Project Settings.


#include "JamPlayerController.h"

void AJamPlayerController::PlayMenuMusic()
{
    StopAndReleaseEvent(GameplayMusicInstance);
    StopAndReleaseEvent(ThemeMusicInstance);
    PlayEvent(MenuMusic);
}

void AJamPlayerController::PlayGameplayMusic()
{
    StopAndReleaseEvent(MenuMusicInstance);
    StopAndReleaseEvent(ThemeMusicInstance);
    PlayEvent(GameplayMusic);
}

void AJamPlayerController::PlayThemeMusic()
{
    StopAndReleaseEvent(MenuMusicInstance);
    StopAndReleaseEvent(GameplayMusicInstance);
    PlayEvent(ThemeMusic);
}

FMOD::Studio::EventInstance* AJamPlayerController::PlayEvent(UFMODEvent* Event)
{
    FFMODEventInstance PlayingInstance = UFMODBlueprintStatics::PlayEvent2D(this, Event, false);
    
    if (PlayingInstance.Instance != nullptr)
    {
        PlayingInstance.Instance->start();
    }
    
    return PlayingInstance.Instance;
}

void AJamPlayerController::StopAndReleaseEvent(FMOD::Studio::EventInstance* Instance)
{
    if (Instance)
    {
        Instance->stop(FMOD_STUDIO_STOP_ALLOWFADEOUT);
        Instance->release();
    }
}
