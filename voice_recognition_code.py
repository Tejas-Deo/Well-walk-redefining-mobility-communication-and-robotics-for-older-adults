import speech_recognition as sr
import time, os, sys, contextlib
from fuzzywuzzy import fuzz


def return_command():

    @contextlib.contextmanager
    def ignoreStderr():
        devnull = os.open(os.devnull, os.O_WRONLY)
        old_stderr = os.dup(2)
        sys.stderr.flush()
        os.dup2(devnull, 2)
        os.close(devnull)
        try:
            yield
        finally:
            os.dup2(old_stderr, 2)
            os.close(old_stderr)


    with ignoreStderr():

        listening = True
        valid_commands = ["Hey Walker Go away", "Hey walker go to the charging dock", "Hey walker Come back", 
        "Hey walker Start the autonomous system mode"]
        
        # Define the minimum confidence threshold for a valid command match
        min_confidence = 70
        print("inside the init function.....")


        while listening:
            print("Inside while loop....")
            with sr.Microphone() as source:
                recognizer = sr.Recognizer()
                recognizer.adjust_for_ambient_noise(source)
                recognizer.dynamic_energy_threshold = 3000

                try:
                    print("Listening...")
                    audio = recognizer.listen(source)
                    response = recognizer.recognize_google(audio)
                    print("WORD SPOKEN: ", response)


                    '''
                    To match the recognized commands to a list of valid commands
                    '''
                    best_match = None
                    best_match_confidence = 0

                    for command in valid_commands:
                        confidence = fuzz.ratio(response.lower(), command.lower())
                        print("Confidence for command {} is {}".format(command, confidence))
                        if confidence > best_match_confidence:
                            best_match = command
                            best_match_confidence = confidence
                            print('BEST MATCH CONFIDENCE: ', best_match_confidence)
                    
                    # to exdcute the command if the confidence level is above a certain threshold
                    if best_match_confidence >= min_confidence:
                        if best_match == "Go away":
                            print("Best Match", best_match)
                        if best_match == "Park":
                            print("Best Match", best_match)
                
                        if best_match == "Come back":
                            print("Best Match", best_match)
                        if best_match == "Start autonomous system mode":
                            print("Best Match", best_match)
                    
                    else:
                        best_match = "Did not recognize that, can you please repeat?"
                    

                except sr.UnknownValueError:
                    #recognizer = sr.Recognizer()
                    #recognizer.dynamic_energy_threshold = 3000  # Increase the energy threshold in case of an exception
                    best_match = "Did not recognize that, can you please repeat?"


                return best_match
                
                
                    
while True:

    command = return_command()
    
    if command is not None:
        print("FINAL COMMAND: ", command)
        print()
    else:
        continue
