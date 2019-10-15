s.boot;

(
SynthDef(\test, {
	|freq=440, amp=0.8|
	var sig, env;
	env = EnvGen.ar(Env.perc(0.01, 0.5), doneAction:2);
	sig = SinOsc.ar(freq);
	//sig = RLPF.ar(sig, 600, 0.5);
	sig = sig * env * amp;
	Out.ar(0, Pan2.ar(sig, 0.0));
}).add;
)

(
s.meter;
s.scope;
)

SerialPort.devices;
~port = SerialPort.new("/dev/ttyACM0", 9600);

(
// we will receive a index from 0 to 48 from Arduino
// and I want to convert it in midinotes on the major scale.
// Here's the function to do it
~getMidiNode = {|index| 36 + Scale.major[index%7] + (((index/7).asInteger)*12)};
)

(
~charArray = [];
~getValues = Routine.new({
	var ascii;
	{
		ascii = ~port.read.asAscii;
		// se il carattere è un digit, aggiungilo ad un array apposito
		if(ascii.isDecDigit, {~charArray = ~charArray.add(ascii)});
		if(ascii == $a , {
			// se il carattere è una a allora significa che abbiamo finito di
			// accumulare digit associati ad un solo valore numerico.
			// ora dobbiamo convertire l'array di caratteri in un integer utilizzabile
			// e, una volta fatto, si svuoti l'array per i cicli successivi
			~val = ~charArray.collect(_.digit).convertDigits;
			~charArray = [];

			Synth(\test, [\freq, ~getMidiNode.value(~val).midicps, \amp, 0.25]);
		});
	}.loop;
}).play;
)

// stop
(
~getValues.stop;
~port.close;
)

