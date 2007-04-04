# $Header: /data/zender/nco_20150216/nco/src/ssdap/swamp_soapslave.py,v 1.2 2007-04-04 03:25:58 wangd Exp $
# Copyright (c) 2007 Daniel L. Wang
from swamp_common import *
from swamp_config import Config 
import cPickle as pickle
import logging
import SOAPpy
import twisted.web.soap as tSoap
import twisted.web.resource as tResource
import twisted.web.server as tServer
import twisted.web.static as tStatic

log = logging.getLogger("SWAMP")

class SimpleJobManager:
    """SimpleJobManager manages slave tasks run on this system.
    We will want to add contexts so that different tasks do not collide in
    files, but it's not needed right now for benchmarking, and will
    complicate debugging"""
    def __init__(self, cfgName=None):
        if cfgName:
            self.config = Config(cfgName)
        else:
            self.config = Config()
        self.config.read()
        
        cfile = logging.FileHandler(self.config.logLocation)
        formatter = logging.Formatter('%(name)s:%(levelname)s %(message)s')
        cfile.setFormatter(formatter)
        log.addHandler(cfile)
        log.setLevel(self.config.logLevel)
        log.info("Swamp slave logging at "+self.config.logLocation)
        self.config.dumpSettings(log, logging.DEBUG)

        self.jobs = {} # dict: tokens -> jobstate
        self.fileMapper = FileMapper("slave%d"%os.getpid(),
                                     self.config.execSourcePath,
                                     self.config.execScratchPath )

        self.localExec = LocalExecutor(NcoBinaryFinder(self.config),
                                       self.fileMapper)

        self.exportPrefix = "http://%s:%d/%s/" % (self.config.slaveHostname,
                                                 self.config.slavePort,
                                                 self.config.slavePubPath)
        self.token = 0
        pass
    def reset(self):
        # Clean up trash from before:
        # - For now, don't worry about checking jobs still in progress
        # - Delete all the physical files we allocated in the file mapper
        self.fileMapper.cleanPhysicals()
        
    def slaveExec(self, pickledCommand):
        cf = CommandFactory(self.config)
        p = cf.unpickleCommand(pickledCommand)
        log.info("received cmd: %s" % (str(p)))
        etoken = self.localExec.launch(p)
        token = self.token + 1
        self.jobs[token] = etoken
        self.token = token
        return token

    def pollState(self, token):
        assert token in self.jobs
        res = self.localExec.poll(self.jobs[token])
        if res is not None:
            return res
        else:
            return None

    def actualToPub(self, f):
        relative = f.split(self.config.execScratchPath + os.sep, 1)[1]
        return self.exportPrefix + relative
    
    def pollOutputs(self, token):
        assert token in self.jobs
        outs = self.localExec.actualOuts(self.jobs[token])
        
        return map(self.actualToPub, outs) 
    

    def startSlaveServer(self):
        #SOAPpy.Config.debug =1
    
        server = SOAPpy.SOAPServer(("localhost", self.config.slavePort))
        server.registerFunction(self.slaveExec)
        server.registerFunction(self.pollState)
        server.registerFunction(self.pollOutputs)
        server.registerFunction(self.reset)
        server.serve_forever()
        pass

    def startTwistedSlaveServer(self):
        from twisted.internet import reactor
        root = tResource.Resource()
        fileResource = tStatic.File(self.config.execScratchPath)
        tStatic.loadMimeTypes() # load from /etc/mime.types
        root.putChild(self.config.slavePubPath, fileResource)
        root.putChild(self.config.slaveSoapPath, TwistedSoapWrapper(self))
        reactor.listenTCP(self.config.slavePort, tServer.Site(root))
        reactor.run()
        pass
    pass # end class SimpleJobManager

class TwistedSoapWrapper(tSoap.SOAPPublisher):
    def __init__(self, jobManager):
        self.jobManager = jobManager
    def soap_reset(self):
        return self.jobManager.reset()
    def soap_slaveExec(self, pickled):
        return self.jobManager.slaveExec(pickled)
    def soap_pollState(self, token):
        return self.jobManager.pollState(token)
    def soap_pollOutputs(self, token):
        return self.jobManager.pollOutputs(token)





def fakeCommand():
    cf = CommandFactory(Config.dummyConfig())
    ins = ["camsom1pdf/camsom1pdf.cam2.h1.0001-01-01-00000.nc"]
    outs = ["out.nc"]
    linenum = 20
    return cf.pickleCommand(cf.newCommand("ncwa", ({},[],ins+outs),(ins,outs), linenum))

def selfTest():
    pass

def clientTest():
    import SOAPpy
    server = SOAPpy.SOAPProxy("http://localhost:8080/SOAP")
    server.reset()
    tok = server.slaveExec(fakeCommand())
    print "submitted, got token: ", tok
    while True:
        ret = server.pollState(tok)
        if ret is not None:
            print "finish, code ", ret
            break
        time.sleep(1)
    print "actual outs are at", server.pollOutputs(tok)

def main():
    selfTest()
    
    jm = SimpleJobManager("slave.conf")
    #jm.startSlaveServer()
    jm.startTwistedSlaveServer()

if __name__ == '__main__':
    main()

