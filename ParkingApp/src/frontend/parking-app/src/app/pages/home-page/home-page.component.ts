import { Component } from '@angular/core';
import { FormsModule } from '@angular/forms';
import { RouterLink } from '@angular/router';
import { CommonModule } from '@angular/common';
import { ApiService } from '../../services/api.service';

@Component({
  selector: 'app-home-page',
  templateUrl: './home-page.component.html',
  styleUrls: ['./home-page.component.scss'],
  standalone: true,
  imports: [CommonModule, FormsModule, RouterLink]
})

export class HomePageComponent {
  email: string = '';
  password: string = '';
  errorMessage: string = '';
  router: any;

  constructor(private apiService: ApiService) {}

  login() {
    this.apiService.login(this.email, this.password).subscribe({
      next: (response: any) => {
        if (response.token) {
          localStorage.setItem('token', response.token);
          this.router.navigate(['/parking-info']).then(() => {
            window.location.reload();
          });
        }
      },
      error: (error) => {
        this.errorMessage = 'Incorrect login or password';
        console.error('Erro no login:', error);
      }
    });
  }
}
