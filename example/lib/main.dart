import 'dart:async';
import 'dart:io';

import 'package:flutter/material.dart';
import 'package:flutter_tools_list/process/process_tools.dart';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatefulWidget {
  const MyApp({super.key});

  @override
  State<MyApp> createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  final List<String> _logs = [];
  HttpServer? _testServer;
  final int _testPort = 8888;

  void _addLog(String message) {
    setState(() {
      _logs.add('[${DateTime.now().toString().substring(11, 19)}] $message');
    });
    print(message);
  }

  Future<void> _testPortCheck() async {
    _logs.clear();
    _addLog('=== Testing Port $_testPort ===');

    // Check initial state
    _addLog('1. Initial check - Port $_testPort in use: ${isPortInUse(_testPort)}');

    try {
      // Start server
      _addLog('\n2. Starting server on port $_testPort...');
      _testServer = await HttpServer.bind(InternetAddress.anyIPv4, _testPort);
      _addLog('   Server started successfully');

      await Future.delayed(const Duration(milliseconds: 500));
      _addLog('   Port $_testPort in use: ${isPortInUse(_testPort)}');

      // Close server
      _addLog('\n3. Closing server...');
      await _testServer?.close();
      _testServer = null;
      _addLog('   Server closed');

      // Check immediately after closing
      await Future.delayed(const Duration(milliseconds: 100));
      bool immediateCheck = isPortInUse(_testPort);
      _addLog('   Immediate check - Port $_testPort in use: $immediateCheck');

      // Check after delays
      await Future.delayed(const Duration(seconds: 1));
      bool after1s = isPortInUse(_testPort);
      _addLog('   After 1s - Port $_testPort in use: $after1s');

      await Future.delayed(const Duration(seconds: 2));
      bool after3s = isPortInUse(_testPort);
      _addLog('   After 3s - Port $_testPort in use: $after3s');

      if (!immediateCheck && !after1s && !after3s) {
        _addLog('\n✓ SUCCESS! Port correctly shows as available after closing.');
        _addLog('  The TIME_WAIT caching issue is FIXED!');
      } else {
        _addLog('\n✗ Issue detected - port still shows as in use after closing');
      }

    } catch (e) {
      _addLog('ERROR: $e');
      if (_testServer != null) {
        await _testServer?.close();
        _testServer = null;
      }
    }
  }

  @override
  void dispose() {
    _testServer?.close();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(title: const Text('Port Check Test - Fix Verification')),
        body: SingleChildScrollView(
          child: Container(
            padding: const EdgeInsets.all(10),
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.stretch,
              children: [
                ElevatedButton(
                  onPressed: _testPortCheck,
                  child: const Text('Run Port Check Test'),
                ),
                const SizedBox(height: 20),
                Container(
                  padding: const EdgeInsets.all(10),
                  decoration: BoxDecoration(
                    color: Colors.black87,
                    borderRadius: BorderRadius.circular(5),
                  ),
                  child: SelectableText(
                    _logs.isEmpty ? 'Click the button to start test...' : _logs.join('\n'),
                    style: const TextStyle(
                      fontFamily: 'Courier',
                      color: Colors.greenAccent,
                      fontSize: 12,
                    ),
                  ),
                ),
              ],
            ),
          ),
        ),
      ),
    );
  }
}
