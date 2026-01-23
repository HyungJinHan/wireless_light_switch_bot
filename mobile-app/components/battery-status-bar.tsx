import { LinearGradient } from "expo-linear-gradient";
import React from "react";
import { StyleSheet, View } from "react-native";
import { ThemedText } from "./themed-text"; // Assuming ThemedText is available

interface BatteryStatusBarProps {
  level: number; // Battery level in percentage (0-100)
}

const BATTERY_WIDTH = 100;

export default function BatteryStatusBar({ level }: BatteryStatusBarProps) {
  const getGradientColors = (
    batteryLevel: number,
  ): readonly [string, string] => {
    if (batteryLevel <= 20) {
      return ["#FF4C4C", "#FF8A8A"]; // Red for low
    } else if (batteryLevel <= 50) {
      return ["#FFD700", "#FFEC8B"]; // Gold/Yellow for medium
    } else {
      return ["#4CAF50", "#8BC34A"]; // Green for high
    }
  };

  const gradientColors = getGradientColors(level);
  const barWidth = (level / 100) * BATTERY_WIDTH; // Calculate numeric width in pixels

  return (
    <View style={styles.container}>
      <View style={styles.batteryOutline}>
        <LinearGradient
          colors={gradientColors}
          start={{ x: 0, y: 0.5 }}
          end={{ x: 1, y: 0.5 }}
          style={[styles.batteryFill, { width: barWidth } as const]}
        />
      </View>
      <ThemedText style={styles.percentageText}>{level}%</ThemedText>
    </View>
  );
}

const styles = StyleSheet.create({
  container: {
    flexDirection: "row",
    alignItems: "center",
  },
  batteryOutline: {
    width: BATTERY_WIDTH, // Fixed width for the battery bar
    height: 25,
    borderWidth: 1,
    borderColor: "#767676",
    borderRadius: 5,
    overflow: "hidden", // Ensures the fill doesn't go outside the outline
    marginRight: 10,
  },
  batteryFill: {
    height: "100%",
    borderRadius: 3, // Slightly less than outline to fit inside
  },
  percentageText: {
    fontSize: 18,
    fontWeight: "bold",
    color: "black",
  },
});
