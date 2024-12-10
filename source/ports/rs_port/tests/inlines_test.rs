#[cfg(test)]
mod tests {
    use metacall::{
        inline::{node, py, ts},
        load,
    };

    #[test]
    fn inlines() {
        // TODO: This is a dirty trick to check if a loader is available,
        // because loaders are lazily loaded, we should improve this in the future

        if load::from_memory("py", "").is_ok() {
            py! {
                print("hello world")
            }
            py! {print("hello world")}
        }

        if load::from_memory("node", "").is_ok() {
            node! {
                console.log("hello world");
            }
            node! {console.log("hello world")}
        }

        if load::from_memory("ts", "").is_ok() {
            ts! {
                console.log("hello world");
            }
            ts! {console.log("hello world")}
        }
    }
}
