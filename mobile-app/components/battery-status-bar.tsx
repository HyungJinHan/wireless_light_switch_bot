import { LinearGradient } from "expo-linear-gradient";
import React from "react";
import { StyleSheet, View } from "react-native";
import { ThemedText } from "./themed-text";

interface BatteryStatusBarProps {
  level: number; // Battery level in percentage (0-100)
}

export default function BatteryStatusBar({ level }: BatteryStatusBarProps) {
  const barWidth = 100; // Fixed width for the battery bar
  const fillWidth = (level / 100) * barWidth;
  const emptyWidth = barWidth - fillWidth; // Calculate the width of the empty part

  return (
    <View style={styles.container}>
      <View style={styles.batteryOutline}>
        {/* Full-width gradient as the background */}
        <LinearGradient
          colors={["#FF4C4C", "#ffbf00", "#4CAF50"]} // Red, Orange, Green
          locations={[0, 0.3, 1]} // Red at 0%, Orange at 30%, Green at 100%
          start={{ x: 0, y: 0.5 }}
          end={{ x: 1, y: 0.5 }}
          style={styles.fullGradientBar}
        />
        {/* An overlay view to mask the unused portion of the gradient */}
        <View style={[styles.emptyOverlay, { width: emptyWidth }]} />
      </View>
      <ThemedText style={styles.percentageText}>{level}%</ThemedText>
    </View>
  );
}

const styles = StyleSheet.create({
  container: {
    flexDirection: "row",
    alignItems: "center",
    marginBottom: 15,
  },
  batteryOutline: {
    width: 130, // Fixed width for the battery bar
    height: 20,
    borderWidth: 1,
    borderColor: "#e8e8e8",
    borderRadius: 5,
    overflow: "hidden", // Ensures the gradient and overlay are clipped
    marginRight: 10,
    flexDirection: "row", // To position gradient and overlay side by side initially
  },
  fullGradientBar: {
    ...StyleSheet.absoluteFillObject, // Make it cover the entire batteryOutline
    borderRadius: 3,
  },
  emptyOverlay: {
    // This will be positioned on top of the gradient, effectively hiding its right portion
    position: "absolute",
    right: 0,
    height: "100%",
    backgroundColor: "#e8e8e8", // This should match the background of the parent card/view
  },
  percentageText: {
    fontSize: 18,
    fontWeight: "bold",
    color: "black",
  },
});
