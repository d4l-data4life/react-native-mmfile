import {useEffect, useRef, useState} from 'react';
import {
  Animated,
  Easing,
  LayoutChangeEvent,
  StyleSheet,
  Text,
  View,
} from 'react-native';

export const MovingRectangle = () => {
  const animationValue = useRef(new Animated.Value(0)).current;
  const colorAnimation = useRef(new Animated.Value(0)).current;
  const [layout, setLayout] = useState({width: 0, height: 0});

  const rectSize =
    layout.width > 0 && layout.height > 0
      ? Math.max(
          40,
          Math.min(layout.width * 0.35, layout.height * 0.9, layout.width - 16),
        )
      : 0;

  const travelDistance = Math.max(0, layout.width - rectSize);

  useEffect(() => {
    if (travelDistance === 0) {
      return;
    }

    animationValue.setValue(0);
    colorAnimation.setValue(0);

    const animation = Animated.loop(
      Animated.parallel([
        Animated.sequence([
          Animated.timing(animationValue, {
            toValue: -travelDistance,
            duration: 600,
            easing: Easing.inOut(Easing.quad),
            useNativeDriver: false,
          }),
          Animated.timing(animationValue, {
            toValue: 0,
            duration: 600,
            easing: Easing.inOut(Easing.quad),
            useNativeDriver: false,
          }),
        ]),
        Animated.sequence([
          Animated.timing(colorAnimation, {
            toValue: 1,
            duration: 600,
            easing: Easing.inOut(Easing.quad),
            useNativeDriver: false,
          }),
          Animated.timing(colorAnimation, {
            toValue: 0,
            duration: 600,
            easing: Easing.inOut(Easing.quad),
            useNativeDriver: false,
          }),
        ]),
      ]),
    );

    animation.start();
    return () => animation.stop();
  }, [animationValue, colorAnimation, travelDistance]);

  const backgroundColor = colorAnimation.interpolate({
    inputRange: [0, 1],
    outputRange: ['blue', 'green'],
  });

  const onLayout = (event: LayoutChangeEvent) => {
    const {width, height} = event.nativeEvent.layout;
    setLayout({width, height});
  };

  return (
    <View style={styles.container} onLayout={onLayout}>
      {rectSize > 0 && (
        <Animated.View
          style={[
            styles.rectangle,
            {
              width: rectSize,
              height: rectSize,
              marginLeft: travelDistance,
              transform: [{translateX: animationValue}],
              backgroundColor,
            },
          ]}>
          <Text
            style={[styles.text, {fontSize: Math.max(10, rectSize * 0.11)}]}>
            It stops when JS Thread blocked
          </Text>
        </Animated.View>
      )}
    </View>
  );
};

const styles = StyleSheet.create({
  container: {
    flex: 1,
    width: '100%',
    justifyContent: 'center',
    overflow: 'hidden',
  },
  rectangle: {
    borderRadius: 10,
    justifyContent: 'center',
    alignItems: 'center',
    padding: 4,
  },
  text: {
    color: 'white',
    fontWeight: 'bold',
    textAlign: 'center',
  },
});
