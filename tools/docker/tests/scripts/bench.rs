fn pairwise_suffix_sum(arr: Vec<f64>) -> f64 {
	let n = arr.len();
	let mut result: f64 = 0.0;

	for i in 0..n {
		for j in i..n {
			result += arr[i] * arr[j];
		}
	}

	result
}
