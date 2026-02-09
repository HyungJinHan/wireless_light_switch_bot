import { LightBlink } from "@/components/light-blink";
import { useFocusEffect } from "@react-navigation/native";
import React, { useCallback, useState } from "react";
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
import apiClient from "../../services/api";

export default function App() {
  const [isEnabled, setIsEnabled] = useState(false);
  const [loading, setLoading] = useState(true);
  const [statusText, setStatusText] = useState("Loading...");
  const colorScheme = useColorScheme();

  // 1. 서버로부터 현재 상태를 가져오는 함수
  const fetchStatus = async () => {
    try {
      const data = await apiClient.getStatus();
      setIsEnabled(data.status === "ON");
      setStatusText(data.status);
    } catch (error) {
      console.error(error);
      setStatusText("연결 실패");
    } finally {
      setLoading(false);
    }
  };

  // 2. 스위치를 토글할 때 실행되는 함수
  const toggleSwitch = async () => {
    const originalState = isEnabled;
    // 낙관적 UI 업데이트
    setIsEnabled(!originalState);
    setStatusText(!originalState ? "ON" : "OFF");

    try {
      await apiClient.toggleSwitch();
      // 서버 전송 성공 시 실제 상태 동기화
      await fetchStatus();
    } catch (error) {
      console.error(error);
      Alert.alert("오류", "스위치 상태 변경에 실패했습니다.");
      // 실패 시 원래 상태로 복구
      setIsEnabled(originalState);
      setStatusText(originalState ? "ON" : "OFF");
    }
  };

  useFocusEffect(
    useCallback(() => {
      fetchStatus();
    }, []),
  );

  const handleTouch = () => {
    setLoading(true);
    fetchStatus();
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

            <Text style={styles.title}>Light Switch Controller</Text>

            <View style={styles.statusContainer}>
              <Text style={styles.statusLabel}>Current Status:</Text>
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
                style={{ transform: [{ scaleX: 1.5 }, { scaleY: 1.5 }] }} // 모바일 앱 맞춤 크기 조절
              />
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
    padding: 40,
    borderRadius: 20,
    width: "90%",
    alignItems: "center",
    shadowColor: "#000",
    shadowOffset: { width: 0, height: 4 },
    shadowOpacity: 0.1,
    shadowRadius: 12,
    elevation: 5, // 안드로이드 그림자
  },
  title: {
    fontSize: 24,
    fontWeight: "bold",
    color: "#1c1e21",
    marginTop: 20,
    marginBottom: 20,
  },
  statusContainer: {
    flexDirection: "row",
    alignItems: "center",
    marginBottom: 10,
  },
  statusLabel: { fontSize: 18, color: "#606770", marginRight: 8 },
  statusValue: { fontSize: 18, fontWeight: "600" },
  onText: { color: "#3678f4" },
  offText: { color: "#f44336" },
  switchWrapper: { margin: 5 },
});
