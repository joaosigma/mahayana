System runtime
=======

For a list of all available variables see engine.runtime.nut

Direct access to all variables is available using the following table:

```
engine.runtime.vars
```

It behaves as a proxy to ::engine.runtime.varGet and ::engine.runtime.varSet, for example:
```
value = engine.runtime.vars["varA"];
engine.runtime.vars["varA"] = value;
```

The list of available functions:

* engine.runtime.varCreate
* engine.runtime.varGet
* engine.runtime.varSet
* engine.runtime.varList
* engine.runtime.exec
* engine.runtime.quit
* engine.runtime.restart
* engine.runtime.forceCrash (only in debug builds)

Profiling support (TODO):
* engine.profiler.recordStart("<path file>")
* engine.profiler.recordStop()
* engine.profiler.sample("nome")
	returns the minimum, current value and maximum value of a sample in an array: [min, cur, max]
* engine.profiler.sampleAvgPeriod("nome", miliseconds)
	returns the average of a sample in the given time period

Also, the following events can be subscribed:

* engine.events.displayInit()
* engine.events.ready()
* engine.events.onKeyPress(keycode)