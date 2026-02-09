import BatteryStatusBar from "@/components/battery-status-bar"; // Import the new component
import { LightBlink } from "@/components/light-blink";
import { useFocusEffect } from "@react-navigation/native";
import React, { useCallback, useEffect, useState } from "react";
import {
  ActivityIndicator,
  Alert,
  StyleSheet,
  Switch,
  Text,
  TouchableWithoutFeedback,
  useColorScheme,
  View,
} from "react-native";
import { SafeAreaView } from "react-native-safe-area-context";
import apiClient from "../services/api";

interface BatteryStatus {
  level: number;
  voltage: number;
}

interface TemperatureStatus {
  temperature: number;
}

export default function App() {
  const [isEnabled, setIsEnabled] = useState(false);
  const [loading, setLoading] = useState(true);
  const [statusText, setStatusText] = useState("Loading...");
  const [batteryStatus, setBatteryStatus] = useState<BatteryStatus | null>(
    null,
  );
  const [batteryLoading, setBatteryLoading] = useState(true);
  const [temperatureStatus, setTemperatureStatus] =
    useState<TemperatureStatus | null>(null);
  const [temperatureLoading, setTemperatureLoading] = useState(true);
  const colorScheme = useColorScheme();

  const getVoltageTextColor = (voltage: number) => {
    if (voltage > 3.7) {
      return "green";
    } else if (voltage >= 3.2) {
      return "orange";
    } else {
      return "red";
    }
  };

  const fetchSwitchStatus = async () => {
    setLoading(true); // Start loading for switch
    try {
      const data = await apiClient.getStatus();
      setIsEnabled(data.status === "ON");
      setStatusText(data.status);
    } catch (error) {
      console.error(error);
      setStatusText("연결 실패");
    } finally {
      setLoading(false); // End loading for switch
    }
  };

  const fetchBatteryStatus = async () => {
    setBatteryLoading(true); // Start loading for battery
    try {
      const data = await apiClient.getBatteryStatus();
      setBatteryStatus(data);
    } catch (err) {
      console.error("Failed to fetch battery status:", err);
      setBatteryStatus(null);
    } finally {
      setBatteryLoading(false); // End loading for battery
    }
  };

  const fetchTemperatureStatus = async () => {
    setTemperatureLoading(true);
    try {
      const data = await apiClient.getTemperature();
      setTemperatureStatus(data);
    } catch (err) {
      console.error("Failed to fetch temperature status:", err);
      setTemperatureStatus(null);
    } finally {
      setTemperatureLoading(false);
    }
  };

  const fetchAllData = async () => {
    await Promise.all([
      fetchSwitchStatus(),
      fetchBatteryStatus(),
      fetchTemperatureStatus(),
    ]);
  };

  const toggleSwitch = async () => {
    const originalState = isEnabled;
    setIsEnabled(!originalState);
    setStatusText(!originalState ? "ON" : "OFF");

    try {
      await apiClient.toggleSwitch();
      await fetchSwitchStatus();
    } catch (error) {
      console.error(error);
      Alert.alert("오류", "스위치 상태 변경에 실패했습니다.");
      setIsEnabled(originalState);
      setStatusText(originalState ? "ON" : "OFF");
    }
  };

  useFocusEffect(
    useCallback(() => {
      fetchAllData();
      // eslint-disable-next-line react-hooks/exhaustive-deps
    }, []),
  );

  useEffect(() => {
    const intervalId = setInterval(() => {
      fetchTemperatureStatus();
    }, 10000); // Poll every 10 seconds

    return () => clearInterval(intervalId); // Cleanup on unmount
  }, []);

  const handleTouch = async () => {
    // Manually refresh switch and battery data only
    await Promise.all([fetchSwitchStatus(), fetchBatteryStatus()]);
  };

  return (
    <SafeAreaView
      style={{
        ...styles.safeArea,
        backgroundColor: colorScheme === "dark" ? "#2e2e2e" : "#f0f2f5",
      }}>
      <TouchableWithoutFeedback onPress={handleTouch}>
        <View style={styles.container}>
          <View style={styles.card}>
            <LightBlink />

            <Text style={styles.title}>Light Switch & Sensors</Text>

            {/* Switch Section */}
            <View style={styles.statusContainer}>
              <Text style={styles.statusLabel}>Switch Status:</Text>
              {loading ? (
                <ActivityIndicator size="small" color="#606770" />
              ) : (
                <Text
                  style={[
                    styles.statusValue,
                    isEnabled ? styles.onText : styles.offText,
                  ]}>
                  {statusText}
                </Text>
              )}
            </View>

            <View style={styles.switchWrapper}>
              <Switch
                trackColor={{ false: "#ccc", true: "#2e2e2e" }}
                thumbColor={"#ffffff"}
                ios_backgroundColor="#ccc"
                onValueChange={toggleSwitch}
                value={isEnabled}
                style={{ transform: [{ scaleX: 1.5 }, { scaleY: 1.5 }] }}
              />
            </View>

            <View style={styles.divider} />

            {/* Battery Section */}
            <Text style={styles.sectionTitle}>Battery Info</Text>
            <View style={styles.batteryStatusRow}>
              {/* New style for battery row */}
              {batteryLoading ? (
                <ActivityIndicator size="small" color="#606770" />
              ) : batteryStatus ? (
                <BatteryStatusBar level={batteryStatus.level} />
              ) : (
                <Text style={styles.errorText}>Error</Text>
              )}
            </View>
            <View style={styles.statusContainer}>
              <Text style={styles.statusLabel}>Voltage:</Text>
              {batteryLoading ? (
                <ActivityIndicator size="small" color="#606770" />
              ) : batteryStatus ? (
                <Text
                  style={[
                    styles.statusValue,
                    { color: getVoltageTextColor(batteryStatus.voltage) },
                  ]}>
                  {batteryStatus.voltage.toFixed(2)}V
                </Text>
              ) : (
                <Text style={styles.errorText}>Error</Text>
              )}
            </View>

            <View style={styles.divider} />

            {/* Temperature Section */}
            <Text style={styles.sectionTitle}>Temperature Info</Text>
            <View style={styles.statusContainer}>
              <Text style={styles.statusLabel}>Current Temp:</Text>
              {temperatureLoading ? (
                <ActivityIndicator size="small" color="#606770" />
              ) : temperatureStatus ? (
                <Text style={styles.statusValue}>
                  {temperatureStatus.temperature.toFixed(2)}°C
                </Text>
              ) : (
                <Text style={styles.errorText}>Error</Text>
              )}
            </View>
          </View>
        </View>
      </TouchableWithoutFeedback>
    </SafeAreaView>
  );
}

const styles = StyleSheet.create({
  safeArea: { flex: 1 },
  container: {
    flex: 1,
    justifyContent: "center",
    alignItems: "center",
    padding: 20,
  },
  card: {
    backgroundColor: "white",
    padding: 30,
    borderRadius: 20,
    width: "90%",
    alignItems: "center",
    shadowColor: "#000",
    shadowOffset: { width: 0, height: 4 },
    shadowOpacity: 0.1,
    shadowRadius: 12,
    elevation: 5,
  },
  title: {
    fontSize: 24,
    fontWeight: "bold",
    color: "#1c1e21",
    marginTop: 20,
    marginBottom: 20,
  },
  sectionTitle: {
    fontSize: 20,
    fontWeight: "bold",
    color: "#1c1e21",
    marginTop: 15,
    marginBottom: 10,
  },
  statusContainer: {
    flexDirection: "row",
    alignItems: "center",
    marginBottom: 10,
  },
  batteryStatusRow: {
    // New style for the battery level display
    flexDirection: "row",
    alignItems: "center",
    marginBottom: 10,
    // Adjust padding/margin if needed to align with other elements
  },
  statusLabel: { fontSize: 18, color: "#606770", marginRight: 8 },
  statusValue: { fontSize: 18, fontWeight: "600" },
  onText: { color: "#3678f4" },
  offText: { color: "#f44336" },
  errorText: { fontSize: 16, color: "red" },
  switchWrapper: { margin: 5 },
  divider: {
    height: 1,
    width: "80%",
    backgroundColor: "#e0e0e0",
    marginVertical: 20,
  },
});
