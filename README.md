## Reference: 
[A SIMPLE BUT EFFICIENT REAL-TIME VOICE ACTIVITY DETECTION ALGORITHM](https://www.eurasip.org/Proceedings/Eusipco/Eusipco2009/contents/papers/1569192958.pdf)

[Dominant Frequency Extraction](https://arxiv.org/pdf/1306.0103.pdf)

[C code](https://github.com/panmasuo/voice-activity-detection)

## Requirements:
```
Eigen 3.3.7
Libsoundfile 1.0.28
```



## Compile:
```
g++ -g ./src/main.cpp -o main -std=c++17 -lstdc++ -lm -lsndfile
```

## Run:
```
./src/main <input_file_path> <output_file_path>
```


