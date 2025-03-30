import { useCallback, useState } from 'react';
import {
  ActivityIndicator,
  SafeAreaView,
  StyleSheet,
  Text,
  TouchableOpacity,
  View,
} from 'react-native';
import { MovingRectangle } from './MovingRectangle';
import { openMmfile, openEncryptedMmfile } from 'react-native-mmfile';
import { MMKV } from 'react-native-mmkv';

import {prepareReactNativeFS, appendReactNativeFS} from './storages/ReactNativeFS';
import {prepareMMKV, appendMMKV} from './storages/MMKV';
import {prepareMmfile, appendMmfile} from './storages/Mmfile';
import {prepareMmfileEncrypted, appendMmfileEncrypted} from './storages/MmfileEncrypted';
import {prepareMMKVEncrypted, appendMMKVEncrypted} from './storages/MMKVEncrypted';

const storage = new MMKV();
const totalSize = 1 * 1024 * 1024; // 1 MB
const keyBuffer: ArrayBuffer = new ArrayBuffer(16);

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

  async function appendMmfileBench(chunkSize = 16) {
    const buffer = new ArrayBuffer(chunkSize);
    let mmapFile = openMmfile('test1.txt');
    const numWrites = totalSize / chunkSize;
    await measureTime(() => {
      for (let i = 0; i < numWrites; i++) {
        mmapFile.append(buffer);
      }
    });
    mmapFile.clear();
    mmapFile.close();
  }

  async function appendEncryptedMmfileBench(chunkSize = 16) {
    const buffer = new ArrayBuffer(chunkSize);
    let key = new Uint8Array([
      0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 
      0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff]);

    let mmapFile = openEncryptedMmfile('test1.txt', key.buffer);
    const numWrites = totalSize / chunkSize;
    await measureTime(() => {
      for (let i = 0; i < numWrites; i++) {
        mmapFile.append(buffer);
      }
    });
    mmapFile.clear();
    mmapFile.close();
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
  // const totalSize = 1024;
  let chunkSize = 16;
  
  async function benchmarkNoAppend(
    chunkSize = 16,
    label: string,
    fn: () => unknown | Promise<unknown>,
  ) {
    const numWrites = totalSize / chunkSize;
    try {
      const buffer = new ArrayBuffer(totalSize);
      const str = String().padStart(totalSize, "*");
      const start = performance.now();
      for (let i = 0; i < numWrites; i++) {
        fn(buffer.slice(0, (i + 1) * chunkSize), str.slice(0, (i + 1) * chunkSize));
      }
      const end = performance.now();
      const diff = end - start;
      const throughputMBs = (totalSize / (diff / 1000)) / (1024 * 1024);
      console.log(`Append "${label}" took ${diff.toFixed(4)} ms, throughput ${throughputMBs.toFixed(2)} MB/s`);
      return diff;
    } catch (e) {
      console.error(`Failed Benchmark "${label}"!`, e);
      return 0;
    }
  }
  
  async function benchmark(
    chunkSize = 16,
    label: string,
    fn: () => unknown | Promise<unknown>,
  ) {
    const numWrites = totalSize / chunkSize;
    try {
      const buffer = new ArrayBuffer(chunkSize);
      const str = String().padStart(chunkSize, "*");
      const start = performance.now();
      for (let i = 0; i < numWrites; i++) {
        fn(buffer, str);
      }
      const end = performance.now();
      const diff = end - start;
      const throughputMBs = (totalSize / (diff / 1000)) / (1024 * 1024);
      console.log(`Append "${label}" took ${diff.toFixed(4)} ms, throughput ${throughputMBs.toFixed(2)} MB/s`);
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
    for (let chunkSize = 16; chunkSize <= 1024*1024; chunkSize = chunkSize * 2) {
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
  }, []);
  
    return (
    <SafeAreaView style={styles.container}>
      <View>
        <Text style={styles.header}>Mmfile Performance Test</Text>
        <View style={styles.resultContainer}>
          <Text style={styles.resultText}>Throughput:</Text>
          <Text style={styles.timeText}>
            {Math.round((totalSize / timeTaken) / (1024 * 1024) * 1000 * 100) / 100} MB/s
          </Text>
          <Text style={styles.resultText}>Time taken:</Text>
          <Text style={styles.timeText}>
            {Math.round((timeTaken / 1000) * 100) / 100} s
          </Text>
        </View>
      </View>

      <MovingRectangle />

      {loading && (
        <ActivityIndicator size="large" color="#6200ee" style={styles.loader} />
      )}

      <View style={styles.buttonContainer}>
        <View style={styles.column}>
          <Text style={styles.columnHeader}>MMKV</Text>
          <TouchableOpacity
            style={styles.button}
            onPress={async () => {
              measureMMKVAppendTimeBench(16)
            }}
          >
            <Text style={styles.buttonText}>Append 1MB in 16B chunks</Text>
          </TouchableOpacity>
          <TouchableOpacity
            style={styles.button}
            onPress={async () => {
              measureMMKVAppendTimeBench(1024);
            }}
          >
            <Text style={styles.buttonText}>Append 1MB in 1KB chunks</Text>
          </TouchableOpacity>
        </View>

        <View style={styles.column}>
          <Text style={styles.columnHeader}>Encrypted Mmfile</Text>
          <TouchableOpacity
            style={styles.button}
            onPress={async () => {
              appendEncryptedMmfileBench(16);
            }}
          >
            <Text style={styles.buttonText}>Append 1MB in 16B chunks</Text>
          </TouchableOpacity>
          <TouchableOpacity
            style={styles.button}
            onPress={async () => {
              appendEncryptedMmfileBench(1024);
            }}
          >
            <Text style={styles.buttonText}>Append 1MB in 1KB chunks</Text>
          </TouchableOpacity>

          <TouchableOpacity
            style={styles.button}
            onPress={async () => {
              runBenchmarks();
            }}
          >
            <Text style={styles.buttonText}>Start Benchmark</Text>
          </TouchableOpacity>
        </View>
      </View>
    </SafeAreaView>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    padding: 20,
    justifyContent: 'space-around',
    backgroundColor: '#f0f4f7',
  },
  header: {
    fontSize: 28,
    fontWeight: 'bold',
    textAlign: 'center',
    marginBottom: 30,
    color: '#333',
  },
  resultContainer: {
    marginBottom: 20,
    padding: 15,
    borderRadius: 10,
    backgroundColor: '#fff',
    shadowColor: '#000',
    shadowOpacity: 0.1,
    shadowRadius: 10,
    shadowOffset: { width: 0, height: 4 },
    elevation: 5,
  },
  resultText: {
    fontSize: 18,
    fontWeight: '600',
    marginVertical: 5,
    color: '#555',
  },
  timeText: {
    fontSize: 20,
    fontWeight: '700',
    color: '#6200ee',
    marginVertical: 5,
  },
  buttonContainer: {
    flexDirection: 'row',
    justifyContent: 'space-between',
  },
  column: {
    flex: 1,
    marginHorizontal: 10,
  },
  columnHeader: {
    fontSize: 20,
    fontWeight: 'bold',
    textAlign: 'center',
    marginBottom: 15,
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
    marginVertical: 20,
  },
});
