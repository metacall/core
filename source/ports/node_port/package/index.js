const addon = 
module.exports = {
    metacall () {
        if(arguments.length == 0) throw Error("No Argument Passed!");

        var functionName = typeof arguments[0] == "string" ? arguments[0] : null
        if(functionName == null) throw Error("Function Name should be of string type");

    },
    
    metacall_load_from_file (filename, arrayOfFileNames) {
        var tag = typeof filename == "string" ? filename : null;
        var array = arrayOfFileNames instanceof  Array ? arrayOfFileNames : null;
        if(tag == null && array == null){
            throw Error("Invalid Arguments, The valid arguments should be a tag string and an Array of strings(filenames)");
        }
        // check how to know when node is in debug mode....
        // make call to Node Addon....
    }
}