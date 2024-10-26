import { Component, Input } from '@angular/core';
import { CommonModule } from '@angular/common';

@Component({
  selector: 'app-parking-info-card',
  standalone: true,
  imports: [CommonModule],
  template: `
    <div class="info-card">
      <h3 class="info-card__title">{{ title }}</h3>
      <div class="info-card__value">
        <ng-container [ngSwitch]="true">
          <span *ngSwitchCase="typeof value === 'boolean'">
            {{ value ? 'Connected' : 'Disconnected' }}
          </span>
          <span *ngSwitchDefault>{{ value }}</span>
        </ng-container>
      </div>
    </div>
  `,
  styles: [`
    .info-card {
      background-color: #ffffff;
      border-radius: 8px;
      padding: 1rem;
      box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
      margin: 0.5rem;
    }

    .info-card__title {
      color: #666;
      font-size: 0.875rem;
      margin-bottom: 0.5rem;
    }

    .info-card__value {
      color: #333;
      font-size: 1.5rem;
      font-weight: bold;
    }
  `]
})
export class ParkingInfoCardComponent {
  @Input() title: string = '';
  @Input() value: string | number | boolean = '';

  get typeof() {
    return typeof this;
  }
}