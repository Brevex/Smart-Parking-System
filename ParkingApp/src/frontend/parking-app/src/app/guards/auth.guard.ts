import { CanActivateFn, Router } from '@angular/router';
import { inject } from '@angular/core';

export const authGuard: CanActivateFn = (route, state) => {
  const router = inject(Router);
  const publicPages = ['/', '/sign-up'];
  const authRequired = !publicPages.includes(state.url);
  const loggedIn = localStorage.getItem('token');

  if (authRequired && !loggedIn) {
    router.navigate(['/']);
    return false;
  }

  return true;
};
