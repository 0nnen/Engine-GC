#include "first_app.h"
#include "Managers/SoundSystemManager.h"
#include "EngineTestCode.h"

#include <iostream>
#include <cstdlib>

int main() {
    SoundSystemManager sound;
    EngineTestCode engineTestCode;

    FMOD::ChannelGroup* mouseGroup;
    sound.createChannelGroup(&mouseGroup);

    engineTestCode.TestCode();

    SoundClass soundSample;
    sound.createSound(&soundSample, "C:\\Users\\polob\\OneDrive\\Bureau\\projet\\Projet13\\engine-mmaqjlkf\\x64\\Debug\\test.wav");
    
    // ajoute le son au groupe "MouseGroup"
    sound.addSoundToGroup(soundSample, mouseGroup);
    
    sound.playSound(soundSample, false);


    /**
     * @brief Fonction principale du programme.
     *
     * Cette fonction cr�e une instance de l'application FirstApp et lance son ex�cution en appelant la m�thode Run().
     *
     * @return EXIT_SUCCESS si l'application s'est termin�e avec succ�s, EXIT_FAILURE sinon.
     */
    lve::FirstApp app{};

    try {
        app.Run();
        
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    sound.releaseSound(soundSample);

    return EXIT_SUCCESS;
}