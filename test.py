import time
Start = time.time()

CSV = open("annual-enterprise-survey-2023-financial-year-provisional-size-bands.csv").read()

Step = CSV.replace("\r\n", "\n")
Step2 = Step.split("\n")
Params = Step2[0].split(",")
Step2.pop(0)
Output = []
for v in Step2:
	OpenQuote = False
	Step3 = []
	Current = ""
	for c in v:
		if c == "\"" and OpenQuote == False:
			OpenQuote = True
		elif c == "\"" and OpenQuote == True:
			OpenQuote = False

		if c == "," and OpenQuote == False:
			Step3.append(Current)
			Current = ""
		else:
			Current += c

	Step3.append(Current)
	
	Output.append({})
	for n,d in enumerate(Step3):
		Output[len(Output) - 1][Params[n]] = d

print(Output[1])
print("Execution Time = " + str(time.time() - Start))
