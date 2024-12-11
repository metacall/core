#[cfg(test)]
mod tests {
    #[test]
    fn test_init() {
        let _metacall = metacall::initialize().unwrap();

        assert!(metacall::is_initialized());
    }
}
