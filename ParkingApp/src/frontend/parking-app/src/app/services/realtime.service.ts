import { Injectable } from '@angular/core';
import { HttpClient } from '@angular/common/http';
import { Observable, map } from 'rxjs';
import { ParkingData, ApiResponse } from '../types/parking.types';

@Injectable({
  providedIn: 'root'
})
export class RealtimeService {
  private DATABASE_URL = '';

  constructor(private http: HttpClient) { }

  getRealtimeData(path: string): Observable<any> {
    return this.http.get<ApiResponse<any>>(`${this.DATABASE_URL}${path}.json`)
      .pipe(
        map(response => {
          // Ensure the response is properly typed based on the path
          switch (path) {
            case '/availableParkingSpaces':
              return Number(response.data);
            case '/lastValidID':
            case '/lastInvalidID':
              return String(response.data);
            case '/invalidAttempts':
            case '/alarmActivations':
              return Number(response.data);
            case '/isWifiConnected':
              return Boolean(response.data);
            default:
              return response.data;
          }
        })
      );
  }

  // Convenience methods with proper typing
  getAvailableParkingSpaces(): Observable<number> {
    return this.getRealtimeData('/availableParkingSpaces');
  }

  getLastValidID(): Observable<string> {
    return this.getRealtimeData('/lastValidID');
  }

  getLastInvalidID(): Observable<string> {
    return this.getRealtimeData('/lastInvalidID');
  }

  getInvalidAttempts(): Observable<number> {
    return this.getRealtimeData('/invalidAttempts');
  }

  getAlarmActivations(): Observable<number> {
    return this.getRealtimeData('/alarmActivations');
  }

  getIsWifiConnected(): Observable<boolean> {
    return this.getRealtimeData('/isWifiConnected');
  }
}