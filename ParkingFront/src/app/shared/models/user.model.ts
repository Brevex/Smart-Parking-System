export class User {
  constructor(
    public token: string,
    public email: string,
    public id?: string,
    public displayName?: string
  ) {}

  get tokenValue(): string {
    return this.token;
  }
}