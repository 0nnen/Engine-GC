#pragma once
#include <string>

class StatusMessage
{
public:
    enum Status
    {
        Send,
        None
    };

    // M�thode pour obtenir l'instance unique de StatusMessage
    static StatusMessage& getInstance() {
        static StatusMessage instance; // Cr�e une seule instance de StatusMessage
        return instance;
    }

    // M�thodes pour modifier l'�tat
    void setStatus(Status newStatus) {
        status_ = newStatus;
    }

    Status getStatus() const {
        return status_;
    }

private:
    StatusMessage() : status_(None) {} // Initialisation par d�faut � None
    Status status_;
};