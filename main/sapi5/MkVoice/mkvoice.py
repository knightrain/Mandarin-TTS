import os
import Tkinter
from Tkinter import *       
import tkFileDialog
import tkMessageBox
import itertools 

class Application(Frame):              
    def __init__(self, master=None):
        Frame.__init__(self, master)   
        self.grid()                    
        self.createWidgets()

    def createWidgets(self):
        Label(self, text='Voice Name:').grid()
        self.voiceNameVar = StringVar()
        self.voiceNameVar.set('NewVoice');
        self.voiceNameEntry = Entry(self, textvariable=self.voiceNameVar, width=30)
        self.voiceNameEntry.grid(row=0, column=1, columnspan=2, padx=5)
        self.listButton = Button(self, text='Mandarin List', width=15,
            command=self.listFile)        
        self.listButton.grid(row=1)
        self.listVar = StringVar()
        self.listEntry = Entry(self, textvariable=self.listVar, width=50)
        self.listEntry.grid(row=1, column=1, columnspan=5, padx=5, pady=5)
        self.VoiceButton = Button(self, text='Voice', width=15,
            command=self.voiceDir)        
        self.VoiceButton.grid(row=2)
        self.voiceVar = StringVar()
        self.VoiceEntry = Entry(self, textvariable=self.voiceVar, width=50)
        self.VoiceEntry.grid(row=2, column=1, columnspan=5, padx=5, pady=5)
        self.mkVoiceButton = Button(self, text='Make Voice', width=15,
            command=self.mkVoice)
        self.mkVoiceButton.grid(row=3, column=2)

    def listFile(self):
        listFile = tkFileDialog.askopenfilename(filetypes=[('list', '*.list')])
        self.listVar.set(listFile)

    def voiceDir(self):
        voice = tkFileDialog.askdirectory(mustexist=False)
        self.voiceVar.set(voice)

    def containsAny(self, seq, aset):
        for item in itertools.ifilter(aset.__contains__, seq):
            return True
        return False

    def mkVoice(self):
        print(os.getcwd())
        if self.containsAny(self.voiceNameVar.get(), " %^!~:\t"):
            tkMessageBox.showerror('Error', 'Illegle symbol %^!~:{space} in Voice Name')
            return
        if not os.path.isfile(self.listVar.get()):
            tkMessageBox.showerror('Error', 'Wrong Mandarin List file path')
            return
        if not os.path.isdir(self.voiceVar.get()):
            tkMessageBox.showerror('Error', 'Wrong voice directory path')
            return
        dictFile = tkFileDialog.asksaveasfilename(filetypes=[('dict', '*.dict')])
        if dictFile:
            cmd = 'mkvoice %s %s %s %s' % (self.voiceNameVar.get(), self.listVar.get(), dictFile, self.voiceVar.get())
            if os.system(cmd) == 0:
                message = '%s is created successfully' % (self.voiceNameVar.get())
                tkMessageBox.showinfo('Successful', message)
        else:
            tkMessageBox.showerror('Error', 'Invalid dictionary file name')
            return

app = Application()
app.master.title("Make Voice")
app.master.resizable(False, False)
app.mainloop()
