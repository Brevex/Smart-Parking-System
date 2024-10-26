import { Component, OnInit } from '@angular/core';
import { Router, RouterLink } from '@angular/router';
import { CommonModule } from '@angular/common';
import { ApiService } from '../../services/api.service';

@Component({
  selector: 'app-header',
  templateUrl: './app-header.component.html',
  styleUrls: ['./app-header.component.scss'],
  standalone: true,
  imports: [CommonModule, RouterLink]
})

export class AppHeaderComponent implements OnInit {
  loggedIn: boolean = false;
  logoLink: string = '/';

  constructor(private router: Router, private apiService: ApiService) { }

  ngOnInit() {
    this.updateLoginStatus();
  }

  updateLoginStatus() {
    this.loggedIn = localStorage.getItem('token') !== null;
    this.logoLink = this.loggedIn ? '/parking-info' : '/';
  }

  logout() {
    this.apiService.logout().subscribe({
      next: () => {
        localStorage.removeItem('token');
        this.loggedIn = false;
        this.router.navigate(['/']);
      },
      error: (error: any) => {
        console.error('Erro ao fazer logout:', error);
      }
    });
  }
}
