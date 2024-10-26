export interface ParkingData {
    availableParkingSpaces: number;
    lastValidID: string;
    lastInvalidID: string;
    invalidAttempts: number;
    alarmActivations: number;
    isWifiConnected: boolean;
}
  
export interface ApiResponse<T> {
    data: T;
    status: string;
}