import { Colors } from "@/constants/theme";
import React, { useEffect, useState } from "react";
import {
  ActivityIndicator,
  Alert,
  StyleSheet,
  Switch,
  Text,
  useColorScheme,
  View,
} from "react-native";
import { SafeAreaView } from "react-native-safe-area-context";

// ⚠️ 포트포워딩한 공인 IP 또는 DDNS 주소로 수정하세요.
const SERVER_URL = "http://172.30.1.92";

export default function App() {
  const [isEnabled, setIsEnabled] = useState(false);
  const [loading, setLoading] = useState(true);
  const [statusText, setStatusText] = useState("Loading...");
  const colorScheme = useColorScheme();

  // 1. 서버로부터 현재 상태를 가져오는 함수 (웹의 updateStatus와 동일)
  const fetchStatus = async () => {
    try {
      const response = await fetch(`${SERVER_URL}/status`);
      if (!response.ok) throw new Error();
      const data = await response.json();

      setIsEnabled(data.status === "ON");
      setStatusText(data.status);
    } catch (error) {
      console.error(error);
      setStatusText("연결 실패");
    } finally {
      setLoading(false);
    }
  };

  // 2. 스위치를 토글할 때 실행되는 함수 (웹의 toggleSwitch와 동일)
  const toggleSwitch = async () => {
    const originalState = isEnabled;
    // 낙관적 UI 업데이트 (사용자 경험을 위해 먼저 바꿈)
    setIsEnabled(!originalState);
    setStatusText(!originalState ? "ON" : "OFF");

    try {
      const response = await fetch(`${SERVER_URL}/toggle`);
      if (!response.ok) throw new Error();

      // 서버 전송 성공 시 실제 상태 동기화
      fetchStatus();
    } catch (error) {
      console.error(error);
      Alert.alert("오류", "스위치 상태 변경에 실패했습니다.");
      // 실패 시 원래 상태로 복구
      setIsEnabled(originalState);
      setStatusText(originalState ? "ON" : "OFF");
    }
  };

  // 앱 실행 시 최초 1회 상태 확인
  useEffect(() => {
    fetchStatus();
  }, []);

  return (
    <SafeAreaView
      style={{
        ...styles.safeArea,
        backgroundColor: Colors[colorScheme ?? "dark"].tint,
      }}>
      <View style={styles.container}>
        <View style={styles.card}>
          <Text style={styles.title}>Light Switch Bot</Text>

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
    marginBottom: 20,
  },
  statusContainer: {
    flexDirection: "row",
    alignItems: "center",
    marginBottom: 20,
  },
  statusLabel: { fontSize: 18, color: "#606770", marginRight: 8 },
  statusValue: { fontSize: 18, fontWeight: "600" },
  onText: { color: "#3678f4" },
  offText: { color: "#f44336" },
  switchWrapper: { padding: 10 },
});
