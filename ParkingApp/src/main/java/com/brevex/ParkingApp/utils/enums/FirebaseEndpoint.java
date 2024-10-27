package com.brevex.ParkingApp.utils.enums;

import lombok.Getter;

@Getter
public enum FirebaseEndpoint {
    SIGN_UP("accounts:signUp"),
    SIGN_IN("accounts:signInWithPassword"),
    DELETE_ACCOUNT("accounts:delete");

    private final String endpoint;

    FirebaseEndpoint(String endpoint) {
        this.endpoint = endpoint;
    }
}
