import { useEffect } from "react";
import Animated, {
  useAnimatedStyle,
  useSharedValue,
  withRepeat,
  withSequence,
  withTiming,
} from "react-native-reanimated";

export function LightBlink() {
  const opacity = useSharedValue(0);

  useEffect(() => {
    opacity.value = withRepeat(
      withSequence(
        withTiming(1, { duration: 20 }),
        withTiming(0, { duration: 200 }),
        withTiming(1, { duration: 150 }),
        withTiming(0, { duration: 90 }),
        withTiming(1, { duration: 150 }),
        withTiming(0, { duration: 200 }),
        withTiming(1, { duration: 110 }),
        withTiming(0, { duration: 150 }),
        withTiming(1, { duration: 20 }),
        withTiming(0, { duration: 300 }),
        withTiming(1, { duration: 500 }),
        withTiming(1, { duration: 5000 }),
        withTiming(0.6, { duration: 500 }),
        withTiming(1, { duration: 5000 }),
      ),
      -1,
      true,
    );
  }, [opacity]);

  const animatedStyle = useAnimatedStyle(() => ({
    opacity: opacity.value,
  }));

  return (
    <Animated.Text
      style={[
        {
          fontSize: 28,
          lineHeight: 32,
          marginTop: -6,
        },
        animatedStyle,
      ]}>
      💡
    </Animated.Text>
  );
}
