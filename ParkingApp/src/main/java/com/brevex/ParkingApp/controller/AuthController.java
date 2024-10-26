package com.brevex.ParkingApp.controller;

import com.brevex.ParkingApp.infra.security.TokenService;
import com.brevex.ParkingApp.model.User;
import com.brevex.ParkingApp.service.FirebaseAuthManager;
import com.brevex.ParkingApp.service.FirebaseTokenService;
import com.brevex.ParkingApp.service.FirebaseUserManager;
import lombok.Getter;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.security.authentication.AuthenticationServiceException;
import org.springframework.web.bind.annotation.*;

@RestController
@RequestMapping("/auth")
@CrossOrigin(origins = "http://localhost:3000")
public class AuthController {
    private static final Logger logger = LoggerFactory.getLogger(AuthController.class);

    private final FirebaseAuthManager firebaseAuthManager;
    private final FirebaseUserManager firebaseUserManager;
    private final FirebaseTokenService firebaseTokenService;
    private final TokenService tokenService;

    @Autowired
    public AuthController(FirebaseAuthManager firebaseAuthManager,
            FirebaseUserManager firebaseUserManager,
            FirebaseTokenService firebaseTokenService,
            TokenService tokenService) {
        this.firebaseAuthManager = firebaseAuthManager;
        this.firebaseUserManager = firebaseUserManager;
        this.firebaseTokenService = firebaseTokenService;
        this.tokenService = tokenService;
    }

    @PostMapping("/create")
    public ResponseEntity<String> createUser(@RequestParam String email, @RequestParam String password) {
        String userId = firebaseUserManager.createUser(email, password);

        return ResponseEntity.ok("User created with ID: " + userId);
    }

    @PostMapping("/login")
    public ResponseEntity<?> login(@RequestParam String email, @RequestParam String password) {
        User user = firebaseAuthManager.authenticateUser(email, password);
        String jwtToken = tokenService.generateToken(user);

        firebaseTokenService.storeIdToken(user.getUsername(), user.getId());

        return ResponseEntity.ok(new AuthResponse(jwtToken));
    }

    @PostMapping("/logout")
    public ResponseEntity<String> logout(@RequestHeader("Authorization") String authHeader) {
        String token = authHeader.substring(7);

        try {
            String email = tokenService.validateToken(token);

            firebaseTokenService.deleteStoredIdToken(email);

            return ResponseEntity.ok("User logged out successfully");
        } catch (AuthenticationServiceException e) {
            logger.error("Invalid or expired token", e);

            return ResponseEntity
                    .status(HttpStatus.UNAUTHORIZED)
                    .body("Invalid or expired token");
        } catch (RuntimeException e) {
            logger.error("Failed to logout user", e);

            return ResponseEntity
                    .status(HttpStatus.INTERNAL_SERVER_ERROR)
                    .body("Failed to logout user");
        }
    }

    @DeleteMapping("/delete")
    public ResponseEntity<String> deleteUser(@RequestHeader("Authorization") String authHeader) {
        String token = authHeader.substring(7);

        try {
            String email = tokenService.validateToken(token);
            String idToken = firebaseTokenService.retrieveStoredIdToken(email);

            if (idToken == null) {
                throw new RuntimeException("Failed to retrieve ID token");
            }

            firebaseUserManager.deleteUser(idToken);
            firebaseTokenService.deleteStoredIdToken(email);

            return ResponseEntity.ok("User deleted successfully");
        } catch (AuthenticationServiceException e) {
            logger.error("Invalid or expired token", e);

            return ResponseEntity
                    .status(HttpStatus.UNAUTHORIZED)
                    .body("Invalid or expired token");
        } catch (RuntimeException e) {
            logger.error("Failed to delete user", e);

            return ResponseEntity
                    .status(HttpStatus.INTERNAL_SERVER_ERROR)
                    .body("Failed to delete user");
        }
    }
}

@Getter
class AuthResponse {
    private final String token;

    public AuthResponse(String token) {
        this.token = token;
    }
}