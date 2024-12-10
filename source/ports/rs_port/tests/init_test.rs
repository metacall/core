#[cfg(test)]
mod tests {
    #[test]
    fn test_init() {
        assert_eq!(metacall::is_initialized(), true);
    }
}
