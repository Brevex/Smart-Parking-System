import { Routes } from '@angular/router';
import { HomePageComponent } from './pages/home-page/home-page.component';
import { SignUpComponent } from './pages/sign-up/sign-up.component';
import { ParkingInfoComponent } from './pages/parking-info/parking-info.component';
import { authGuard } from './guards/auth.guard';

export const routes: Routes = [
  { path: '', component: HomePageComponent },
  { path: 'sign-up', component: SignUpComponent },
  { path: 'parking-info', component: ParkingInfoComponent, canActivate: [authGuard] },
  { path: '**', redirectTo: '' }
];