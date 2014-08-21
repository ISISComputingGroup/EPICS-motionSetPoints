# Assumes motionSetPoints is running using test.cmd 
# and talking to a motor simulator (assumes $(MYPVPREFIX)MTR01)
# Note that this test waits for puts to complete and takes the motor for a couple of drives so allow ~1min

import unittest
import os
import time
from shutil import copyfile
from epics import PV

class testMotionSetPoints(unittest.TestCase):
	'''Test stuff'''
	prefix = ''
	DELAY = 2
	
	def setUp(self):
		try:
			PVPREFIX = os.environ['MYPVPREFIX']
		except KeyError:
			PVPREFIX = 'NDW1298:sjb99183:'
		self.prefix = PVPREFIX + 'LKUP:TEST:'
		self.pvReset = self.getPV('RESET')
		self.pvFilter1 = self.getPV('FILTER1')
		self.pvFilter2 = self.getPV('FILTER2')
		self.pvFilterOut = self.getPV('FILTER:OUT')
		self.pvPositions = self.getPV('POSITIONS')
		self.pvPosnSp = self.getPV('POSN:SP')

		
	def getPV(self, name):
		print('Creating ' + self.prefix + name)
		return PV(self.prefix + name)
		
	def test_filtering(self):
		copyfile('../../iocBoot/iocmotionSetPoints/test1.txt', '../../iocBoot/iocmotionSetPoints/test.txt')
		self.pvReset.put(1)
		self.pvFilter1.put('')
		self.pvFilter2.put('')
		time.sleep(self.DELAY)
		positions = self.pvPositions.get()
		print(str(positions))
		self.assertEqual(positions[0], 'A_A_A')
		self.assertEqual(positions[1], 'A_B_B')
		self.assertEqual(positions[5], 'END')
		
		self.pvFilter1.put('B')
		time.sleep(self.DELAY)
		positions = self.pvPositions.get()
		print(str(positions))
		self.assertEqual(positions[0], 'B_A_A')
		self.assertEqual(positions[3], 'END')
		
		self.pvFilter2.put('A_|B_A|B_C')
		time.sleep(self.DELAY)
		positions = self.pvPositions.get()
		print(str(positions))
		self.assertEqual(positions[1], 'B_C_B')
		self.assertEqual(positions[2], 'END')
		
		self.pvPosnSp.put('B_C_B')
		time.sleep(self.DELAY)
		self.assertEqual(self.pvFilterOut.get(), 'X')
		
	def test_stuff(self):
		copyfile('../../iocBoot/iocmotionSetPoints/test2.txt', '../../iocBoot/iocmotionSetPoints/test.txt')
		self.pvReset.put(1)
		self.pvFilter1.put('')
		self.pvFilter2.put('')
		time.sleep(self.DELAY)
		positions = self.pvPositions.get()
		print(str(positions))
		self.assertEqual(positions[0], 'A_A_A')
		self.assertEqual(positions[4], 'B_C_B')
		self.assertEqual(positions[5], 'B_C_C')
		self.assertEqual(positions[6], 'END')

		self.pvPosnSp = self.getPV('POSN:SP')
		pvCoord1 = self.getPV('COORD1')
		
		self.pvPosnSp.put('A_A_A')
		time.sleep(self.DELAY)
		self.assertAlmostEqual(pvCoord1.get(), 1)
		
		pvPosnSpRbv = self.getPV('POSN:SP:RBV')
		self.assertEqual(pvPosnSpRbv.get(), 'A_A_A')
		
		pvStationary = self.getPV('STATIONARY')
		for i in range(20):
			stationary = pvStationary.get()
			if stationary==1:
				break
			print('Moving')
			time.sleep(self.DELAY)

		self.assertEqual(stationary, 1)
		
		pvPositioned = self.getPV('POSITIONED')
		self.assertEqual(pvPositioned.get(), 1)
		
		pvCoord1Rbv = self.getPV('COORD1:RBV')
		self.assertAlmostEqual(pvCoord1Rbv.get(), 1)
		
		pvPosn = self.getPV('POSN')
		self.assertEqual(pvPosn.get(), 'A_A_A')
		
		pvTolerence = self.getPV('TOLERENCE')
		pvTolerence.put(-1)
		time.sleep(self.DELAY)
		self.assertEqual(pvPositioned.get(), 0)
		pvTolerence.put(1)
		time.sleep(self.DELAY)
		self.assertEqual(pvPositioned.get(), 1)
		
		self.pvPosnSp.put('INVALID')
		time.sleep(self.DELAY)
		self.assertEqual(pvPosnSpRbv.get(), 'A_A_A')

		# A long drive
		self.pvPosnSp.put('B_C_C')
		time.sleep(self.DELAY)
		self.assertEqual(pvStationary.get(), 0)
		self.assertEqual(pvPositioned.get(), 0)
		self.assertEqual(self.pvFilterOut.get(), 'Y')
		self.assertAlmostEqual(pvCoord1.get(), 20)
		self.assertNotEqual(pvPosn.get(), 'B_C_C')

		for i in range(100):
			coordRbv = pvCoord1Rbv.get()
			stationary = pvStationary.get()
			if stationary==1:
				break
			print('Moving')
			self.assertLess(coordRbv, 20)
			time.sleep(self.DELAY)
		
		self.assertAlmostEqual(pvCoord1Rbv.get(), 20)
		self.assertEqual(pvStationary.get(), 1)
		self.assertEqual(pvPositioned.get(), 1)
		self.assertEqual(pvPosn.get(), 'B_C_C')
