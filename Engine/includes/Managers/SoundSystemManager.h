#pragma once

#include <fmod.hpp> // Inclure la biblioth�que FMOD
#include <cstddef> // Pour inclure la d�finition de NULL
#include <iostream> // Pour les entr�es/sorties standard

typedef FMOD::Sound* SoundClass; // D�finition du type pour faciliter l'utilisation de la classe Sound de FMOD

/**
 * @brief Classe SoundSystemManager : G�re le syst�me audio utilisant la biblioth�que FMOD
 */
class SoundSystemManager
{
public:
    /**
     * @brief Constructeur par d�faut de la classe SoundSystemManager
     */
    SoundSystemManager();

    /**
     * @brief Cr�e un objet Sound � partir d'un fichier audio sp�cifi�
     * @param pSound Pointeur vers l'objet Sound cr��
     * @param pathAudio Chemin du fichier audio � charger
     */
    void createSound(SoundClass* pSound, const char* pathAudio);

    /**
     * @brief Joue un son sp�cifi�
     * @param pSound Pointeur vers l'objet Sound � jouer
     * @param isPlay Bool�en indiquant si le son doit �tre jou� imm�diatement (par d�faut, false)
     */
    void playSound(SoundClass pSound, bool isPlay = false);

    /**
     * @brief Lib�re les ressources utilis�es par un son sp�cifi�
     * @param pSound Pointeur vers l'objet Sound � lib�rer
     */
    void releaseSound(SoundClass pSound);

    /**
     * @brief Cr�e un groupe de canaux pour le syst�me audio
     * @param channelGroup Pointeur vers le groupe de canaux cr��
     */
    void createChannelGroup(FMOD::ChannelGroup** channelGroup);

    /**
     * @brief Ajoute un son � un groupe de canaux sp�cifi�
     * @param pSound Pointeur vers l'objet Sound � ajouter au groupe de canaux
     * @param channelGroup Pointeur vers le groupe de canaux auquel ajouter le son
     */
    void addSoundToGroup(SoundClass pSound, FMOD::ChannelGroup* channelGroup);

    /**
     * @brief Attache un groupe de canaux � un port sp�cifi� pour le rendu audio.
     * @param portType Type de port auquel attacher le groupe de canaux (par ex. FMOD_PORT_TYPE_PS3_SPDIF).
     * @param portIndex Index du port auquel attacher le groupe de canaux.
     * @param channelGroup Pointeur vers le groupe de canaux � attacher.
     * @param passThru Indique si le signal doit �tre achemin� directement sans traitement (par d�faut, false).
     * @return R�sultat de l'op�ration d'attache du groupe de canaux au port.
     */
    FMOD_RESULT attachChannelGroupToPort(FMOD_PORT_TYPE portType, FMOD_PORT_INDEX portIndex, FMOD::ChannelGroup* channelGroup, bool passThru = false);

    /**
     * @brief D�tache un groupe de canaux d'un port auquel il �tait pr�c�demment attach�.
     * @param channelGroup Pointeur vers le groupe de canaux � d�tacher du port.
     * @return R�sultat de l'op�ration de d�tachement du groupe de canaux du port.
     */
    FMOD_RESULT detachChannelGroupFromPort(FMOD::ChannelGroup* channelGroup);

    /**
     * @brief D�finit les propri�t�s de r�verb�ration pour une instance de r�verb�ration sp�cifi�e.
     * @param instance Instance de r�verb�ration pour laquelle d�finir les propri�t�s.
     * @param prop Pointeur vers les propri�t�s de r�verb�ration � d�finir.
     * @return R�sultat de l'op�ration de d�finition des propri�t�s de r�verb�ration.
     */
    FMOD_RESULT setReverbProperties(int instance, const FMOD_REVERB_PROPERTIES* prop);

    /**
     * @brief Obtient les propri�t�s de r�verb�ration pour une instance de r�verb�ration sp�cifi�e.
     * @param instance Instance de r�verb�ration pour laquelle obtenir les propri�t�s.
     * @param prop Pointeur vers les propri�t�s de r�verb�ration � remplir avec les valeurs obtenues.
     * @return R�sultat de l'op�ration de r�cup�ration des propri�t�s de r�verb�ration.
     */
    FMOD_RESULT getReverbProperties(int instance, FMOD_REVERB_PROPERTIES* prop);

private:
    FMOD::System* system; // Pointeur vers l'objet syst�me FMOD
    FMOD::ChannelGroup* channelGroup; // Pointeur vers le groupe de canaux utilis�
};
