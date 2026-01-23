import ParallaxScrollView from "@/components/parallax-scroll-view";
import { ThemedText } from "@/components/themed-text";
import { ThemedView } from "@/components/themed-view";
import { StyleSheet, ActivityIndicator, Alert } from "react-native";
import { useEffect, useState } from "react";
import apiClient from "../../services/api"; // Correct relative path
import BatteryStatusBar from "@/components/battery-status-bar"; // Import the new component

interface BatteryStatus {
  level: number;
  voltage: number;
}

export default function BatteryScreen() {
  const [batteryStatus, setBatteryStatus] = useState<BatteryStatus | null>(null);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);

  const fetchBatteryStatus = async () => {
    try {
      setLoading(true);
      setError(null);
      const data = await apiClient.getBatteryStatus();
      setBatteryStatus(data);
    } catch (err) {
      console.error("Failed to fetch battery status:", err);
      setError("Failed to fetch battery status.");
      Alert.alert("Error", "Failed to fetch battery status. Please check the connection.");
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    fetchBatteryStatus();
  }, []);

  return (
    <ParallaxScrollView
      headerBackgroundColor={{ light: "#D0D0D0", dark: "#353636" }}
      headerImage={<ThemedText type="title" style={styles.headerTitle}>Battery Status</ThemedText>}
    >
      <ThemedView style={styles.titleContainer}>
        <ThemedText type="title">Battery Information</ThemedText>
      </ThemedView>

      {loading && (
        <ThemedView style={styles.statusContainer}>
          <ActivityIndicator size="large" color="#606770" />
          <ThemedText style={styles.statusText}>Loading battery status...</ThemedText>
        </ThemedView>
      )}

      {error && (
        <ThemedView style={styles.statusContainer}>
          <ThemedText style={styles.errorText}>Error: {error}</ThemedText>
        </ThemedView>
      )}

      {batteryStatus && !loading && !error && (
        <ThemedView style={styles.infoContainer}>
          <ThemedText type="defaultSemiBold" style={styles.label}>Battery Level:</ThemedText>
          <BatteryStatusBar level={batteryStatus.level} />

          <ThemedText type="defaultSemiBold" style={styles.label}>Voltage:</ThemedText>
          <ThemedText style={styles.value}>{batteryStatus.voltage.toFixed(2)}V</ThemedText>
        </ThemedView>
      )}
    </ParallaxScrollView>
  );
}

const styles = StyleSheet.create({
  headerTitle: {
    paddingLeft: 20,
    fontSize: 30,
    fontWeight: 'bold',
  },
  titleContainer: {
    flexDirection: "row",
    gap: 8,
    marginBottom: 20,
  },
  statusContainer: {
    alignItems: 'center',
    justifyContent: 'center',
    paddingVertical: 20,
  },
  statusText: {
    marginTop: 10,
    fontSize: 16,
    color: "#606770",
  },
  errorText: {
    marginTop: 10,
    fontSize: 16,
    color: "red",
  },
  infoContainer: {
    padding: 20,
    backgroundColor: 'white',
    borderRadius: 10,
    shadowColor: "#000",
    shadowOffset: { width: 0, height: 2 },
    shadowOpacity: 0.1,
    shadowRadius: 8,
    elevation: 3,
  },
  label: {
    fontSize: 18,
    marginBottom: 5,
    color: "#1c1e21",
  },
  value: {
    fontSize: 24,
    fontWeight: 'bold',
    marginBottom: 15,
    color: "#3678f4",
  },
});
