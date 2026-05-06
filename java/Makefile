llama2fu.jar: MANIFEST.MF obj/Llama2Java.class
	jar cfm $@ $< -C obj .

obj/Llama2Java.class: Llama2Java.java transpiled/Llama2Cli.java
	mkdir -p $(@D) && javac -d $(@D) $< transpiled/*.java

transpiled/Llama2Cli.java: ../llama2.fu ../llama2cli.fu
	mkdir -p $(@D) && fut -o $@ $^

clean:
	$(RM) llama2fu.jar obj/*.class transpiled/*.java
