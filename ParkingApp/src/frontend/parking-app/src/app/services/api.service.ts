import { Injectable } from '@angular/core';
import { HttpClient, HttpHeaders } from '@angular/common/http';

@Injectable({
  providedIn: 'root'
})

export class ApiService {
  private baseUrl = 'http://localhost:8080/auth';

  constructor(private http: HttpClient) { }

  login(email: string, password: string) {
    const params = { email, password };
    return this.http.post(`${this.baseUrl}/login`, null, { params });
  }

  createUser(email: string, password: string) {
    const params = { email, password };
    return this.http.post(`${this.baseUrl}/create`, null, { params });
  }

  logout() {
    const token = localStorage.getItem('token');
    const headers = new HttpHeaders().set('Authorization', `Bearer ${token}`);

    return this.http.post(`${this.baseUrl}/logout`, null, { headers });
  }
}
