import { Component } from '@angular/core';
import { FormsModule } from '@angular/forms';
import { RouterLink } from '@angular/router';
import { CommonModule } from '@angular/common';
import { ApiService } from '../../services/api.service';

@Component({
  selector: 'app-sign-up',
  templateUrl: './sign-up.component.html',
  styleUrls: ['./sign-up.component.scss'],
  standalone: true,
  imports: [CommonModule, FormsModule, RouterLink]
})

export class SignUpComponent {
  email: string = '';
  confirmEmail: string = '';
  password: string = '';
  confirmPassword: string = '';
  errorMessage: string = '';
  router: any;

  constructor(private apiService: ApiService) {}

  createUser() {
    if (this.email !== this.confirmEmail) {
      this.errorMessage = 'Emails do not match';
      return;
    }

    if (this.password !== this.confirmPassword) {
      this.errorMessage = 'Passwords do not match';
      return;
    }

    this.apiService.createUser(this.email, this.password).subscribe({
      next: () => {
        this.router.navigate(['/parking-info']);
      },
      error: (error) => {
        console.error('Erro ao criar usuário:', error);
      }
    });
  }
}
