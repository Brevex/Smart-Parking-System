package com.brevex.ParkingApp.service;

import com.brevex.ParkingApp.model.User;
import com.brevex.ParkingApp.utils.exceptions.AuthenticationServiceException;
import com.brevex.ParkingApp.utils.enums.FirebaseEndpoint;
import io.github.cdimascio.dotenv.Dotenv;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.core.ParameterizedTypeReference;
import org.springframework.http.HttpEntity;
import org.springframework.http.HttpHeaders;
import org.springframework.http.HttpMethod;
import org.springframework.http.ResponseEntity;
import org.springframework.stereotype.Service;
import org.springframework.web.client.HttpStatusCodeException;
import org.springframework.web.client.RestTemplate;

import java.util.HashMap;
import java.util.Map;

@Service
public class FirebaseAuthManager extends FirebaseSetupService {
    @Autowired
    public FirebaseAuthManager(RestTemplate restTemplate, Dotenv dotenv) {
        super(restTemplate, dotenv);
    }

    public User authenticateUser(String email, String password) {
        try {
            String url = buildUrl(FirebaseEndpoint.SIGN_IN);

            Map<String, String> request = new HashMap<>();
            request.put("email", email);
            request.put("password", password);
            request.put("returnSecureToken", "true");

            HttpHeaders headers = new HttpHeaders();
            HttpEntity<Map<String, String>> entity = new HttpEntity<>(request, headers);

            ResponseEntity<Map<String, Object>> response = restTemplate.exchange(
                    url,
                    HttpMethod.POST,
                    entity,
                    new ParameterizedTypeReference<>() {
                    });

            Map<String, Object> responseBody = response.getBody();

            if (response.getStatusCode().is2xxSuccessful() && responseBody != null) {
                String idToken = (String) responseBody.get("idToken");

                return new User(idToken, email, null);
            } else {
                throw new AuthenticationServiceException("Invalid email or password");
            }
        } catch (HttpStatusCodeException e) {
            throw new AuthenticationServiceException(
                    "Failed to authenticate user: " + e.getResponseBodyAsString());
        } catch (Exception e) {
            throw new AuthenticationServiceException("Failed to authenticate user");
        }
    }
}