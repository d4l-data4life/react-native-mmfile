import {MMKV} from 'react-native-mmkv';

const storage = new MMKV({
  id: 'encrypted-mmkv-storage',
  encryptionKey: 'hunter2',
});

storage.clearAll();

const key = 'k';

export function prepareMMKVEncrypted() {
  storage.clearAll();
}

export function appendMMKVEncrypted(buffer: ArrayBuffer, str: string) {
  storage.set(key, str);
}
