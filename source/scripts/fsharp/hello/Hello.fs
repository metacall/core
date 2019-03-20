namespace Scripts

type IJump =
    abstract member Jump : unit -> int

type SuperJump() =
    interface IJump with
        member __.Jump () = 2

type TinyJump() =
    interface IJump with
        member __.Jump () = 1

type JumpMaster() =
    static member SuperJump () : int =
        let sj = SuperJump() :> IJump
        sj.Jump ()

    static member TinyJump () =
        let sj = TinyJump() :> IJump
        sj.Jump ()

type Program() =
    static member SayHello () = printfn "%s" "HELLO"
    static member Say text = printfn "%s" text
    static member Sum a b = a + b
    static member Concat (a:string) (b:string) = a + b
