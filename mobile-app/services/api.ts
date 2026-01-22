const SERVER_URL = "http://172.30.1.96";

const apiClient = {
  async request(endpoint: string, options?: RequestInit) {
    const url = `${SERVER_URL}${endpoint}`;
    const response = await fetch(url, options);

    if (!response.ok) {
      const errorBody = await response.text();
      console.error(`API Error: ${response.status} ${response.statusText}`, errorBody);
      throw new Error(`Request failed with status ${response.status}`);
    }

    const contentType = response.headers.get("content-type");
    if (contentType && contentType.includes("application/json")) {
      return response.json();
    }

    return;
  },

  getStatus() {
    return this.request('/status');
  },

  toggleSwitch() {
    return this.request('/toggle');
  },

  getBatteryStatus() {
    return this.request('/battery');
  },

  getTemperature() {
    return this.request('/temperature');
  },
};

export default apiClient;