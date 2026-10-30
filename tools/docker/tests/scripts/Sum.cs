using System;

// MetaCall's C# loader (Roslyn) exposes top-level local functions as callable

int sum_cs(int left, int right) => left + right;

string greet_cs(string name) => $"Hello {name} from C#!";
