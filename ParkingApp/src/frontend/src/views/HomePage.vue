<template>
  <div class="form-container">
    <form @submit.prevent="login" class="form">
      <router-link to="/" class="userIcon">
        <img src="@/assets/icons/user.png" alt="User Icon"/>
      </router-link>

      <input v-model="email" type="email" placeholder="Email" required class="input" />
      <input v-model="password" type="password" placeholder="Password" required class="input" />

      <p v-if="errorMessage" class="error-message">{{ errorMessage }}</p>
      <button type="submit" class="button">Sign In</button>

      <div class="form-text">
        <p>Don't have an account?</p>
        <router-link to="/sign-up" class="form-link">Sign up here</router-link>
      </div>
    </form>
  </div>
</template>

<script>
import { ref } from 'vue';
import { useRouter } from 'vue-router';
import api from '../services/api';

export default
{
  setup()
  {
    const email = ref('');
    const password = ref('');
    const errorMessage = ref('');
    const router = useRouter();

    const login = async () => {
      try
      {
        const response = await api.post('/login', null, {
          params: {email: email.value, password: password.value}
        });

        if (response.status === 200 && response.data.token)
        {
          localStorage.setItem('token', response.data.token);
          await router.push('/parking-info');
          window.location.reload();
        }
      }
      catch (error)
      {
        errorMessage.value = 'Incorrect login or password';
        console.error('Erro no login:', error);
      }
    };
    return {email, password, login, errorMessage};
  }
};
</script>

<style scoped>
.error-message
{
  color: red;
  margin-top: 1rem;
}

.form
{
  display: flex;
  flex-direction: column;
  align-items: center;
  background-color: #17052A;
  border-radius: 10px;
  margin-bottom: 1.5rem;
  padding: 2rem;
}

.input
{
  padding: 1rem;
  margin-top: 1rem;
  border-radius: 5px;
  border: 1px solid #ccc;
  background-color: white;
  width: 14rem;
}

.button
{
  padding: 1rem;
  border: none;
  border-radius: 5px;
  background-color: #080010;
  font-weight: bolder;
  color: white;
  cursor: pointer;
  margin-top: 1.5rem;
}

.button:hover
{
  background-color: #330066;
}

.form-text
{
  color: white;
  margin-top: 1rem;
  text-align: center;
}

.form-link
{
  color: #fff;
  font-weight: bold;
}
</style>
