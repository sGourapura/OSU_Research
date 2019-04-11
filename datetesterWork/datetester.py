import datetime

now = datetime.datetime.now()

f = open("runDate.txt", "w")
f.write("Current date and time: "+now.strftime("%Y-%m-%d %H:%M"))
