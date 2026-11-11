#!/usr/bin/env python3
"""Check the compiled C++ CLI, independent expectations and Go compatibility."""
import argparse
import csv
import io
import json
import random
import subprocess
import tempfile
from collections import deque
from pathlib import Path

parser = argparse.ArgumentParser()
parser.add_argument("--cpp", required=True)
parser.add_argument("--go", required=True)
args = parser.parse_args()
cpp = str(Path(args.cpp).resolve())
go = str(Path(args.go).resolve())
rng = random.Random(505)
processes = 0
scenarios = 0

def invoke(binary, options, data=""):
    global processes
    processes += 1
    return subprocess.run([binary, *options], input=data, text=True, encoding="utf-8",
                          stdout=subprocess.PIPE, stderr=subprocess.PIPE, timeout=30)

def check(task, data, expected=None, invalid=False, method=None):
    global scenarios
    scenarios += 1
    options = ["--task", task]
    if method:
        options += ["--method", method]
    outputs = []
    for binary in (cpp, go):
        result = invoke(binary, options, data)
        if invalid:
            assert result.returncode != 0, (task, data[:150], binary, result.stdout)
            assert not result.stdout, (task, "partial output on invalid input")
        else:
            assert result.returncode == 0, (task, data[:150], binary, result.stderr)
            if expected is not None:
                assert result.stdout == expected, (task, binary, expected[:150], result.stdout[:150])
            outputs.append(result.stdout)
    if not invalid:
        assert outputs[0] == outputs[1], (task, outputs)
        return outputs[0]

def section(name, body):
    body()
    print("PASS " + name, flush=True)

def examples():
    root = Path(__file__).resolve().parents[2] / "examples"
    for task, file, expected in [
        ("1", "task1_possible.txt", "Возможно\nC B A\n"),
        ("1", "task1_cycle.txt", "Невозможно\n"),
        ("3", "task3.txt", "4\n"),
        ("4", "task4.txt", "2\n"),
        ("4", "task4_second.txt", "4\n"),
        ("5", "task5.txt", "Исправленное дерево (preorder): 2 1 # # 4 3 # # #\nInorder: 1 2 3 4\n"),
        ("6.2", "task6.txt", "A-0\nB-2: A C\nC-0\nD-0\nE-4: A B C D\n"),
        ("7", "task7_first.txt", "2\n"),
        ("7", "task7.txt", "5 -1\n"),
    ]:
        check(task, (root / file).read_text(), expected)

def graphs():
    for sample in range(150):
        n = rng.randrange(1, 25)
        edges = [(a, b) for a in range(n) for b in range(n)
                 if rng.randrange(15) == 0 and (sample % 2 == 0 or a > b)]
        names = [f"T{i}" for i in range(n)]
        degree = [0] * n
        children = [[] for _ in range(n)]
        for a, b in edges:
            degree[a] += 1
            children[b].append(a)
        pending = deque(i for i in range(n) if degree[i] == 0)
        finished = 0
        while pending:
            current = pending.popleft()
            finished += 1
            for child in children[current]:
                degree[child] -= 1
                if degree[child] == 0:
                    pending.append(child)
        data = f"{n} {len(edges)}\n" + " ".join(names) + "\n"
        data += "\n".join(f"{names[a]} {names[b]}" for a, b in edges)
        output = check("1", data)
        assert output.splitlines()[0] == ("Возможно" if finished == n else "Невозможно")
        if finished == n:
            order = output.splitlines()[1].split()
            assert sorted(order) == sorted(names)
            position = {name: i for i, name in enumerate(order)}
            assert all(position[names[b]] < position[names[a]] for a, b in edges)
    n = 10000
    data = f"{n} {n-1}\n" + " ".join(map(str, range(n))) + "\n"
    data += "\n".join(f"{i} {i-1}" for i in range(1, n))
    check("1", data, "Возможно\n" + " ".join(map(str, range(n))) + "\n")

def genomes():
    for _ in range(80):
        a = "".join(rng.choices("ABCD", k=rng.randrange(1, 60)))
        b = "".join(rng.choices("ABCD", k=rng.randrange(1, 60)))
        reference = {b[i:i+2] for i in range(len(b)-1)}
        expected = sum(a[i:i+2] in reference for i in range(len(a)-1))
        check("3", a + " " + b, str(expected) + "\n")
    check("3", "A" * 100000 + " AA", "99999\n")

def stress():
    dictionary = ["cAnnot", "cannOt", "fOund", "pAge", "тЕст", "теСт", "вОда"]
    words = ["cannot", "cAnnot", "cannOt", "PAGE", "pAge", "x", "X", "XX",
             "тЕст", "теСт", "ТЕст", "вОда", "водА", "слОво", "словО"]
    for _ in range(50):
        text = rng.choices(words, k=rng.randrange(1, 80))
        known = {word.lower() for word in dictionary}
        correct = set(dictionary)
        expected = sum(sum(c.isupper() for c in word) != 1 or
                       (word.lower() in known and word not in correct) for word in text)
        check("4", str(len(dictionary)) + "\n" + "\n".join(dictionary) + "\n" + " ".join(text),
              str(expected) + "\n")
    # The Lab 1 C++ array must support the same large dictionary as Go.
    dictionary = [f"w{i}A" for i in range(70000)]
    check("4", "70000\n" + "\n".join(dictionary) + "\nw0A w69999A unknown", "1\n")

def trees():
    for tree in ["4 2 1 # # 3 # # 6 5 # # 7 # #", "1 # 2 # 3 # 4 # #",
                 "4 3 2 1 # # # # #"]:
        original = tree.split()
        positions = [i for i, token in enumerate(original) if token != "#"]
        inorder = sorted(int(original[i]) for i in positions)
        expected = "Исправленное дерево (preorder): " + tree + "\nInorder: "
        expected += " ".join(map(str, inorder)) + "\n"
        for a in range(len(positions)):
            for b in range(a + 1, len(positions)):
                tokens = original[:]
                tokens[positions[a]], tokens[positions[b]] = tokens[positions[b]], tokens[positions[a]]
                check("5", " ".join(tokens), expected)
    # Highly skewed input exercises iterative parsing, recovery and destruction.
    n = 30000
    values = list(range(n))
    values[0], values[-1] = values[-1], values[0]
    data = " ".join(f"{value} #" for value in values) + " #"
    expected = "Исправленное дерево (preorder): " + " ".join(f"{i} #" for i in range(n)) + " #\n"
    expected += "Inorder: " + " ".join(map(str, range(n))) + "\n"
    check("5", data, expected)

def employees():
    for _ in range(40):
        n = rng.randrange(1, 35)
        names = [f"E{i:02}" for i in range(n)]
        parents = [rng.randrange(i, n) for i in range(n)]
        pairs = [(names[i], names[parents[i]]) for i in range(n)]
        pairs += pairs[:rng.randrange(n)]
        rng.shuffle(pairs)
        descendants = {name: [] for name in names}
        for i in range(n):
            current = i
            while parents[current] != current:
                current = parents[current]
                descendants[names[current]].append(names[i])
        expected = ""
        for name in names:
            values = sorted(descendants[name])
            expected += f"{name}-{len(values)}"
            if values:
                expected += ": " + " ".join(values)
            expected += "\n"
        data = str(len(pairs)) + "\n" + "\n".join(a + " " + b for a, b in pairs)
        check("6.2", data, expected, method="linear")
        check("6.2", data, expected, method="double")

def caches():
    for _ in range(75):
        capacity = rng.randrange(0, 10)
        reference = {}
        order = []
        operations = []
        results = []
        for i in range(100):
            key = rng.randrange(-8, 15)
            if rng.randrange(2):
                value = rng.randrange(-1000, 1000)
                operations.append(f"SET {key} {value}")
                if capacity:
                    reference[key] = value
                    if key in order:
                        order.remove(key)
                    order.insert(0, key)
                    if len(order) > capacity:
                        del reference[order.pop()]
            else:
                operations.append(f"GET {key}")
                results.append(str(reference.get(key, -1)))
                if key in reference:
                    order.remove(key)
                    order.insert(0, key)
        check("7", f"{capacity} {len(operations)} " + " ".join(operations), " ".join(results) + "\n")
    check("7", "1 3 SET -9223372036854775808 9223372036854775807 GET -9223372036854775808 GET 0",
          "9223372036854775807 -1\n")

def files():
    global scenarios
    with tempfile.TemporaryDirectory() as temporary:
        path = Path(temporary) / "sets.json"
        reference = {"special": ["line\n", "q\"x", "\\", "😀", "я"], "empty": None}
        path.write_text(json.dumps(reference))
        for binary in (cpp, go):
            for value in reference["special"]:
                # Elements with spaces are not accepted by the query grammar.
                if "\n" not in value:
                    result = invoke(binary, ["--file", str(path), "--query", "SET_AT special " + value])
                    assert result.returncode == 0 and result.stdout == "true\n", result
        states = {name: set(values or []) for name, values in reference.items()}
        for i in range(80):
            binary = cpp if i % 2 else go
            name = rng.choice(["special", "empty", "new"])
            value = rng.choice(["a", "b", "яблоко", "😀", '"quote"', "back\\slash"])
            operation = rng.choice(["SETADD", "SETDEL", "SET_AT", "PRINT"])
            query = f"{operation} {name}" + ("" if operation == "PRINT" else " " + value)
            before = path.read_bytes()
            result = invoke(binary, ["--file", str(path), "--query", query])
            assert result.returncode == 0, result.stderr
            expected_set = states.setdefault(name, set())
            if operation == "SETADD":
                expected_set.add(value)
            elif operation == "SETDEL":
                expected_set.discard(value)
            elif operation == "SET_AT":
                assert result.stdout == ("true\n" if value in expected_set else "false\n")
                assert path.read_bytes() == before
            else:
                assert result.stdout == " ".join(sorted(expected_set)) + "\n"
                assert path.read_bytes() == before
            actual = json.loads(path.read_text())
            assert all(set(values or []) == states[key] for key, values in actual.items())
            scenarios += 1
        for invalid in ["broken", "null", "[]", '{"x":[1]}', '{"x":[]} trailing', '{"x":["abc"]']:
            path.write_text(invalid)
            before = path.read_bytes()
            for binary in (cpp, go):
                result = invoke(binary, ["--file", str(path), "--query", "SETADD a 1"])
                assert result.returncode != 0
                assert path.read_bytes() == before
        path.write_text('{"a":["1"]}')
        for query in ["BAD a", "SETADD a", "SET_AT a", "PRINT a 1"]:
            for binary in (cpp, go):
                before = path.read_bytes()
                result = invoke(binary, ["--file", str(path), "--query", query])
                assert result.returncode != 0
                assert path.read_bytes() == before

def malformed():
    for task, data in [
        ("1", "-1 0"), ("1", "1 1 A A B"), ("1", "2 0 A A"),
        ("3", "AB aB"), ("3", "AB"), ("4", "-1"), ("4", "1 word word"),
        ("5", "#"), ("5", "1 # #"), ("5", "1 #"), ("5", "1 # # 2"),
        ("5", "1 3 # # 2 # #"), ("5", "2 2 # # 2 # #"),
        ("6.2", "-1"), ("6.2", "2 A B B A"), ("6.2", "2 A B A C"),
        ("7", "-1 0"), ("7", "2 1 BAD 1"), ("7", "1 1 SET 1"),
        ("7", "1 1 GET 9223372036854775808"), ("7", "1 1 GET +-1"),
    ]:
        check(task, data, invalid=True)
    for binary in (cpp, go):
        for options in [["--method", "bad", "--task", "3"], ["--file", "x"], ["--task", "unknown"]]:
            assert invoke(binary, options).returncode != 0
        result = invoke(binary, [], "3\nABBACAB BCABB\nEND\n0\n")
        assert result.returncode == 0 and "\n4\n" in result.stdout

def experiments():
    results = []
    for binary in (cpp, cpp, go):
        result = invoke(binary, ["--task", "6.1"])
        assert result.returncode == 0, result.stderr
        rows = list(csv.DictReader(io.StringIO(result.stdout)))
        assert len(rows) == 12
        results.append(rows)
        for row in rows:
            assert int(row["keys"]) > 0
            assert all(int(row[field]) >= int(row["keys"])
                       for field in ("insert_probes", "hit_probes", "miss_probes", "delete_probes"))
    for a, b in zip(results[0], results[1]):
        assert all(a[key] == b[key] for key in a if not key.endswith("_ns"))
    for a, b in zip(results[0], results[2]):
        if a["dataset"] == "same_bucket":
            assert all(a[key] == b[key] for key in a if not key.endswith("_ns")), (a, b)

for name, body in [
    ("provided examples", examples), ("random graphs and long DAG", graphs),
    ("random genomes and maximum length", genomes), ("stress dictionary and Unicode", stress),
    ("all BST swaps and deep tree", trees), ("hierarchies in both hash modes", employees),
    ("random LRU model", caches), ("shared JSON files and failed writes", files),
    ("malformed inputs and menu", malformed), ("hash experiment reproducibility", experiments),
]:
    section(name, body)
print(f"PASS: {scenarios} scenarios, {processes} process runs; C++ and Go agree")
