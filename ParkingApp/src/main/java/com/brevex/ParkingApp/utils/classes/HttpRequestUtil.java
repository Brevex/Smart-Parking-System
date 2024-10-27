package com.brevex.ParkingApp.utils.classes;

import org.springframework.http.HttpEntity;
import org.springframework.http.HttpHeaders;

import java.util.HashMap;
import java.util.Map;

public class HttpRequestUtil {
    public static HttpEntity<Map<String, String>> createRequestEntity(Map<String, String> body, HttpHeaders headers) {
        return new HttpEntity<>(body, headers);
    }

    public static HttpEntity<Map<String, String>> createAuthRequestEntity(String email, String password,
            HttpHeaders headers) {
        Map<String, String> body = new HashMap<>();
        body.put("email", email);
        body.put("password", password);
        body.put("returnSecureToken", "true");

        return createRequestEntity(body, headers);
    }
}
