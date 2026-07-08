import { useCallback, useState } from 'react';
import {
  ActivityIndicator,
  Alert,
  StyleSheet,
  Text,
  TouchableOpacity,
  View,
} from 'react-native';
import { MovingRectangle } from './MovingRectangle';
import { Mmfile } from '@d4l/react-native-mmfile';
import { MMKV } from 'react-native-mmkv';

import {prepareReactNativeFS, appendReactNativeFS} from './storages/ReactNativeFS';
import {prepareMMKV, appendMMKV} from './storages/MMKV';
import {prepareMmfile, appendMmfile} from './storages/Mmfile';
import {prepareMmfileEncrypted, appendMmfileEncrypted} from './storages/MmfileEncrypted';
import {prepareMMKVEncrypted, appendMMKVEncrypted} from './storages/MMKVEncrypted';
import {SafeAreaProvider, SafeAreaView} from 'react-native-safe-area-context';

const storage = new MMKV();

export default function App() {
  const [timeTaken, setTimeTaken] = useState<number>(0);
  const [loading, setLoading] = useState<boolean>(false);

  const measureTime = useCallback(async function <T>(fn: () => T): Promise<T> {
    setLoading(true);
    const start = performance.now();
    const res = await fn();
    const end = performance.now();
    const _timeTaken = end - start;
    console.log('Time taken :', _timeTaken, 'ms');
    setTimeTaken(_timeTaken);
    setLoading(false);
    return res;
  }, []);

  async function appendEncryptedMmfileBench(chunkSize = 16) {
    try {
      const buffer = new ArrayBuffer(chunkSize);
      let key = new Uint8Array([
        0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
        0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff]);

      let mmapFile = Mmfile.openEncryptedMmfile('test1.txt', key.buffer);
      const numWrites = totalSize / chunkSize;
      await measureTime(() => {
        for (let i = 0; i < numWrites; i++) {
          mmapFile.append(buffer);
        }
      });
      mmapFile.clear();
      mmapFile.close();
    } catch (e) {
      console.log('error', e);
    }
  }

  const measureMMKVAppendTimeBench = async (chunkSize = 16) => {
    const buffer = new ArrayBuffer(totalSize);

    const numWrites = totalSize / chunkSize;
    const key = 'test1';
    await measureTime(() => {
      for (let i = 0; i < numWrites; i++) {
        storage.set(key, buffer.slice(0, (i + 1) * chunkSize));
      }
    });
  };

  const totalSize = 1 * 1024 * 1024; // 1 MB

  async function benchmarkNoAppend(
    chunkSize = 16,
    label: string,
    fn: (buffer: ArrayBuffer, str: string) => void,
  ) {
    const numWrites = totalSize / chunkSize;
    try {
      const buffer = new ArrayBuffer(totalSize);
      const str = String().padStart(totalSize, '*');
      const start = performance.now();
      for (let i = 0; i < numWrites; i++) {
        fn(
          buffer.slice(0, (i + 1) * chunkSize),
          str.slice(0, (i + 1) * chunkSize),
        );
      }
      const end = performance.now();
      const diff = end - start;
      const throughputMBs = totalSize / (diff / 1000) / (1024 * 1024);
      console.log(
        `Append "${label}" took ${diff.toFixed(
          4,
        )} ms, throughput ${throughputMBs.toFixed(2)} MB/s`,
      );
      return diff;
    } catch (e) {
      console.error(`Failed Benchmark "${label}"!`, e);
      return 0;
    }
  }

  async function benchmark(
    chunkSize = 16,
    label: string,
    fn: ((buffer: ArrayBuffer, str: string) => Promise<void>) | ((buffer: ArrayBuffer) => void),
  ) {
    const numWrites = totalSize / chunkSize;
    try {
      const buffer = new ArrayBuffer(chunkSize);
      const str = String().padStart(chunkSize, '*');
      const start = performance.now();
      for (let i = 0; i < numWrites; i++) {
        fn(buffer, str);
      }
      const end = performance.now();
      const diff = end - start;
      const throughputMBs = totalSize / (diff / 1000) / (1024 * 1024);
      console.log(
        `Append "${label}" took ${diff.toFixed(
          4,
        )} ms, throughput ${throughputMBs.toFixed(2)} MB/s`,
      );
      return diff;
    } catch (e) {
      console.error(`Failed Benchmark "${label}"!`, e);
      return 0;
    }
  }

  async function waitForGC(): Promise<void> {
    // Wait for Garbage Collection to run. We give a 500ms delay.
    return new Promise(r => setTimeout(r, 500));
  }

  const runBenchmarks = useCallback(async () => {
    console.log('Running Benchmark in 3... 2... 1...');
    for (
      let chunkSize = 16;
      chunkSize <= 1024 * 1024;
      chunkSize = chunkSize * 2
    ) {
      console.log('chunkSize: ' + chunkSize);
      await waitForGC();
      await prepareMMKV();
      await benchmarkNoAppend(chunkSize, 'MMKV', appendMMKV);
      await waitForGC();
      await prepareMMKVEncrypted();
      await benchmarkNoAppend(chunkSize, 'MMKV Encrypt', appendMMKVEncrypted);
      await waitForGC();
      await prepareMmfile();
      await benchmark(chunkSize, 'Mmfile', appendMmfile);
      await waitForGC();
      await prepareMmfileEncrypted();
      await benchmark(chunkSize, 'Mmfile Encrypt', appendMmfileEncrypted);
      // await waitForGC();
      // await benchmarkNoAppend(chunkSize, 'AsyncStorage', appendAsyncStorage);
      await waitForGC();
      await prepareReactNativeFS();
      await benchmark(chunkSize, 'ReactNativeFS', appendReactNativeFS);
    }
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, []);

    return (
      <SafeAreaProvider>
        <SafeAreaView style={styles.container}>
          <View style={styles.topSection}>
            <Text style={styles.header}>Mmfile Performance Test</Text>
            <View style={styles.resultContainer}>
              <View style={styles.resultColumn}>
                <Text style={styles.resultText}>Throughput:</Text>
                <Text style={styles.timeText}>
                  {Math.round(
                    (totalSize / timeTaken / (1024 * 1024)) * 1000 * 100,
                  ) / 100}{' '}
                  MB/s
                </Text>
              </View>
              <View style={styles.resultColumn}>
                <Text style={styles.resultText}>Time taken:</Text>
                <Text style={styles.timeText}>
                  {Math.round((timeTaken / 1000) * 100) / 100} s
                </Text>
              </View>
            </View>
          </View>

          <View style={styles.animationSection}>
            <MovingRectangle />
            {loading && (
              <ActivityIndicator
                size="large"
                color="#6200ee"
                style={styles.loader}
              />
            )}
          </View>

          <View style={styles.buttonContainer}>
            <View style={styles.column}>
              <Text style={styles.columnHeader}>MMKV</Text>
              <TouchableOpacity
                style={styles.button}
                onPress={async () => {
                  measureMMKVAppendTimeBench(16);
                }}>
                <Text style={styles.buttonText}>Append 1MB in 16B chunks</Text>
              </TouchableOpacity>
              <TouchableOpacity
                style={styles.button}
                onPress={async () => {
                  measureMMKVAppendTimeBench(1024);
                }}>
                <Text style={styles.buttonText}>Append 1MB in 1KB chunks</Text>
              </TouchableOpacity>

              <TouchableOpacity
                style={styles.button}
                onPress={async () => {
                  console.log('readDir...');
                  try {
                    let f = Mmfile.openMmfile('test1.txt');
                    const buffer = new ArrayBuffer(1234);
                    f.clear();
                    f.append(buffer);
                    f.close();

                    const files = await Mmfile.readDir('');
                    console.log('readDir()', files);
                    // await Mmfile.unlink("");
                    const files2 = Mmfile.readDirSync('');
                    console.log('readDirSync()', files2);

                    // show popup with files
                    Alert.alert('Files: ' + JSON.stringify(files2));
                  } catch (e) {
                    console.error('Error reading directory:', e);
                  }
                  console.log('readDir done');
                }}>
                <Text style={styles.buttonText}>Read Directory</Text>
              </TouchableOpacity>
            </View>

            <View style={styles.column}>
              <Text style={styles.columnHeader}>Encrypted Mmfile</Text>
              <TouchableOpacity
                style={styles.button}
                onPress={async () => {
                  appendEncryptedMmfileBench(16);
                }}>
                <Text style={styles.buttonText}>Append 1MB in 16B chunks</Text>
              </TouchableOpacity>
              <TouchableOpacity
                style={styles.button}
                onPress={async () => {
                  appendEncryptedMmfileBench(1024);
                }}>
                <Text style={styles.buttonText}>Append 1MB in 1KB chunks</Text>
              </TouchableOpacity>

              <TouchableOpacity
                style={styles.button}
                onPress={async () => {
                  runBenchmarks();
                }}>
                <Text style={styles.buttonText}>Start Benchmark</Text>
              </TouchableOpacity>
            </View>
          </View>
        </SafeAreaView>
      </SafeAreaProvider>
    );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    padding: 16,
    backgroundColor: '#f0f4f7',
  },
  topSection: {
    flexShrink: 0,
  },
  animationSection: {
    flex: 1,
    minHeight: 0,
  },
  header: {
    fontSize: 28,
    fontWeight: 'bold',
    textAlign: 'center',
    marginBottom: 30,
    color: '#333',
  },
  resultContainer: {
    flexDirection: 'row',
    padding: 12,
    borderRadius: 10,
    backgroundColor: '#fff',
    shadowColor: '#000',
    shadowOpacity: 0.1,
    shadowRadius: 10,
    shadowOffset: { width: 0, height: 4 },
    elevation: 5,
  },
  resultColumn: {
    flex: 1,
    alignItems: 'center',
  },
  resultText: {
    fontSize: 18,
    fontWeight: '600',
    marginBottom: 4,
    color: '#555',
  },
  timeText: {
    fontSize: 18,
    fontWeight: '700',
    color: '#6200ee',
    textAlign: 'center',
  },
  buttonContainer: {
    flexShrink: 0,
    flexDirection: 'row',
    justifyContent: 'space-between',
  },
  column: {
    flex: 1,
    marginHorizontal: 6,
  },
  columnHeader: {
    fontSize: 18,
    fontWeight: 'bold',
    textAlign: 'center',
    marginBottom: 8,
    color: '#6200ee',
  },
  button: {
    backgroundColor: '#6200ee',
    paddingVertical: 12,
    paddingHorizontal: 20,
    borderRadius: 30,
    marginBottom: 10,
    alignItems: 'center',
    shadowColor: '#6200ee',
    shadowOpacity: 0.3,
    shadowRadius: 10,
    shadowOffset: { width: 0, height: 4 },
    elevation: 5,
  },
  buttonText: {
    color: '#fff',
    fontSize: 16,
    fontWeight: '600',
    textAlign: 'center',
  },
  loader: {
    ...StyleSheet.absoluteFillObject,
    justifyContent: 'center',
    alignItems: 'center',
  },
});
