package com.brevex.ParkingApp.service;

import com.brevex.ParkingApp.utils.classes.HttpRequestUtil;
import com.brevex.ParkingApp.utils.enums.FirebaseEndpoint;
import io.github.cdimascio.dotenv.Dotenv;
import jakarta.annotation.PostConstruct;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.HttpEntity;
import org.springframework.http.HttpHeaders;
import org.springframework.http.MediaType;
import org.springframework.stereotype.Service;
import org.springframework.web.client.RestTemplate;

import java.util.Map;

@Service
public abstract class FirebaseSetupService {
    protected final String firebaseApiKey;
    protected final RestTemplate restTemplate;
    protected HttpHeaders headers;

    @Autowired
    public FirebaseSetupService(RestTemplate restTemplate, Dotenv dotenv) {
        this.restTemplate = restTemplate;
        this.firebaseApiKey = dotenv.get("FIREBASE_API_KEY");
    }

    @PostConstruct
    public void init() {
        this.headers = new HttpHeaders();
        this.headers.setContentType(MediaType.APPLICATION_JSON);
    }

    protected String buildUrl(FirebaseEndpoint endpoint) {
        return "https://identitytoolkit.googleapis.com/v1/"
                + endpoint.getEndpoint()
                + "?key=" + firebaseApiKey;
    }

    protected HttpEntity<Map<String, String>> createRequestEntity(Map<String, String> body) {
        return HttpRequestUtil.createRequestEntity(body, headers);
    }

    protected HttpEntity<Map<String, String>> createAuthRequestEntity(String email, String password) {
        return HttpRequestUtil.createAuthRequestEntity(email, password, headers);
    }
}
