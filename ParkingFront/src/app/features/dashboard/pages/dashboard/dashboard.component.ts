import { Component, OnInit, OnDestroy } from '@angular/core';
import { DataService } from '../../../../core/services/data.service';
import { Subscription } from 'rxjs';

@Component({
  selector: 'app-dashboard',
  templateUrl: './dashboard.component.html',
  styleUrls: ['./dashboard.component.scss']
})
export class DashboardComponent implements OnInit, OnDestroy {
  realtimeData: any;
  loading = false;
  private dataSubscription!: Subscription;

  constructor(private dataService: DataService) {}

  ngOnInit(): void {
    this.loading = true;
    this.dataSubscription = this.dataService.getRealtimeData().subscribe(
      (data: any) => {
        this.realtimeData = data;
        this.loading = false;
      },
      (error: any) => {
        console.error('Erro ao obter dados:', error);
        this.loading = false;
      }
    );
  }

  ngOnDestroy(): void {
    if (this.dataSubscription) {
      this.dataSubscription.unsubscribe();
    }
  }
}
