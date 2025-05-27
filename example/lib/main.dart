import 'dart:async';

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
  late int sumResult;
  late Future<int> sumAsyncResult;

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(title: const Text('Native Packages')),
        body: SingleChildScrollView(
          child: Container(
            padding: const EdgeInsets.all(10),
            child: Column(
              children: [
                MaterialButton(
                  onPressed: () async {
                    print(await getPidByPath(24340));
                  },
                  child: Text('获取'),
                ),
              ],
            ),
          ),
        ),
      ),
    );
  }
}
