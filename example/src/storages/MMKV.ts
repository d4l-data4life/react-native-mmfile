import {MMKV} from 'react-native-mmkv';

const storage = new MMKV();

const key = 'k';

export function prepareMMKV() {
  storage.clearAll();
}

export function appendMMKV(buffer: ArrayBuffer, str: string) {
  storage.set(key, str);
}
