import { Component, Input } from '@angular/core';

@Component({
  selector: 'app-parking-info-card',
  templateUrl: './parking-info-card.component.html',
  styleUrls: ['./parking-info-card.component.scss'],
  standalone: true
})
export class ParkingInfoCardComponent {
  @Input() title: string = '';
  @Input() value: any;
}