package com.brevex.ParkingApp.service;

import io.github.cdimascio.dotenv.Dotenv;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.HttpEntity;
import org.springframework.http.HttpHeaders;
import org.springframework.http.ResponseEntity;
import org.springframework.stereotype.Service;
import org.springframework.web.client.RestTemplate;

@Service
public class FirebaseTokenService {
    private final String firebaseDatabaseUrl;
    private final RestTemplate restTemplate;

    @Autowired
    public FirebaseTokenService(Dotenv dotenv, RestTemplate restTemplate) {
        this.firebaseDatabaseUrl = dotenv.get("DATABASE_URL");
        this.restTemplate = restTemplate;
    }

    public void storeIdToken(String email, String idToken) {
        String url = String.format(
                "%s/tokens/%s.json",
                firebaseDatabaseUrl,
                email.replace(".", ","));

        HttpHeaders headers = new HttpHeaders();
        headers.set("Content-Type", "application/json");

        HttpEntity<String> request = new HttpEntity<>(
                String.format("\"%s\"", idToken),
                headers);
        restTemplate.put(url, request);
    }

    public String retrieveStoredIdToken(String email) {
        String url = String.format(
                "%s/tokens/%s.json",
                firebaseDatabaseUrl,
                email.replace(".", ","));

        ResponseEntity<String> response = restTemplate.getForEntity(url, String.class);

        String body = response.getBody();
        if (body != null && !body.equals("null")) {
            return body.replace("\"", "");
        }
        return null;
    }

    public void deleteStoredIdToken(String email) {
        String url = String.format(
                "%s/tokens/%s.json",
                firebaseDatabaseUrl,
                email.replace(".", ","));
        restTemplate.delete(url);
    }
}
