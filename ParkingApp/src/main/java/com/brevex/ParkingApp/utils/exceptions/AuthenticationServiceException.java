package com.brevex.ParkingApp.utils.exceptions;

public class AuthenticationServiceException extends RuntimeException {
    public AuthenticationServiceException(String message) {
        super(message);
    }
}
