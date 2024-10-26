import { Component, OnInit, OnDestroy } from '@angular/core';
import { CommonModule } from '@angular/common';
import { RealtimeService } from '../../services/realtime.service';
import { ParkingInfoCardComponent } from '../../components/parking-info-card/parking-info-card.component';
import { Subscription, interval } from 'rxjs';

@Component({
  selector: 'app-parking-info',
  templateUrl: './parking-info.component.html',
  styleUrls: ['./parking-info.component.scss'],
  standalone: true,
  imports: [CommonModule, ParkingInfoCardComponent]
})
export class ParkingInfoComponent implements OnInit, OnDestroy {
  availableParkingSpaces: number = 0;
  lastValidID: string = '';
  lastInvalidID: string = '';
  invalidAttempts: number = 0;
  alarmActivations: number = 0;
  isWifiConnected: boolean = false;

  private subscriptions: Subscription[] = [];

  constructor(private realtimeService: RealtimeService) {}

  ngOnInit() {
    this.loadAllData();
    const refreshSubscription = interval(5000)
      .subscribe(() => this.loadAllData());
    this.subscriptions.push(refreshSubscription);
  }

  ngOnDestroy() {
    this.subscriptions.forEach(sub => sub.unsubscribe());
  }

  private loadAllData() {
    this.subscriptions.push(
      this.realtimeService.getAvailableParkingSpaces()
        .subscribe(data => this.availableParkingSpaces = data)
    );

    this.subscriptions.push(
      this.realtimeService.getLastValidID()
        .subscribe(data => this.lastValidID = data)
    );

    this.subscriptions.push(
      this.realtimeService.getLastInvalidID()
        .subscribe(data => this.lastInvalidID = data)
    );

    this.subscriptions.push(
      this.realtimeService.getInvalidAttempts()
        .subscribe(data => this.invalidAttempts = data)
    );

    this.subscriptions.push(
      this.realtimeService.getAlarmActivations()
        .subscribe(data => this.alarmActivations = data)
    );

    this.subscriptions.push(
      this.realtimeService.getIsWifiConnected()
        .subscribe(data => this.isWifiConnected = data)
    );
  }
}