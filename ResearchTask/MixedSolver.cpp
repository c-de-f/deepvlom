#include "stdafx.h"
#include <ctime>
#include "Solver.h"

#pragma once

float MixedSolver::GetCriticalPath(std::vector<Task*> act)
{


	for (auto i : act)
	{
		//std::cout << i->getName() << ' ';
		i->forceCalculating();
		i->setStatus(::DONE);
		setStatuses();
		//addSuccessors(i);
	}

	//for (auto &i : p->tasks) { (&i.second)->setEarlyStart(0); if (i.second.getStatus() == ::DONE) (&i.second)->setEarlyStart((&i.second)->start()); };
#ifdef VALIDATE
	validate1("_");
#endif // VALIDATE
	 float rcp = partialEarlySolve();
#ifdef VALIDATE
	int a;
	if (a = validate2("_") > 0) { /*for (auto &k : p->tasks) std::cout << k.first << ' ' << k.second.start() << ' ' << k.second.getStatus() << '\n';*/ std::cout << "validator1 says " << a << '\n'; exit(0); }
#endif // VALIDATE

	//rcp = 0;

	//std::cout << rcp << '\n';
	for (auto i : act)
	{
		//std::cout << i->getName() << ' ';
		//i->setStart(0);
		for (auto & j : (i)->assignments)
		{
			j->resource->deleteTask(j);
		}
		i->setStatus(TaskStatus::ACTIVE);
		//std::cout << i->getStatus() << '!';

	}
	setStatuses();
	for (auto &l = p->tasks.begin(); l != p->tasks.end(); l++) if (l->second.getStatus() != TaskStatus::DONE) l->second.setStart(0);
	return rcp;
}


float MixedSolver::bound(int _depth)
{
	std::vector<Task*> act = {};
	for (auto &i = p->tasks.begin(); i != p->tasks.end(); i++)
	{
		Task *t = &i->second;
		
		if (t->getStatus() == ::ACTIVE) { /*std::cout << t->getName() << ' '; */ act.push_back(t); }
		
	}
	//std::cout << '\n';
	//std::cout << act.size() << '\n';
	float rcp;
	int ctr = 0;

	if (!act.size()) { /*std::cout << "Here\n";*/ return p->makespan(); }
	//	if (_depth > ctr) _depth = ctr;
	if ((_depth == 0)) {
		if (h == NULL) 
		{ 
			switch (param) {
				case 0: rcp = GetCriticalPath(act); break; // cr path
				case 1: rcp = p->makespan(); break; // 1st
				case 2:  //Intensity
				{
					int sum = 0; 
					for (auto &t : p->tasks) if (t.second.getStatus() == ::DONE) {
						 sum += t.second.sourceUsage();
					}
					
					rcp = p->makespan() / sum;
					//std::cout << rcp << '\n';
					break;
				}
			}
		}
		else
		{
			//std::cout << "Voila\n";
			Problem tmp_p1 = *p;
			//validate1(".!.");
			int c = 0;
			while (haveActiveTasks()) {
				c++; //{ std::cout << "Calculating " << c << "/" << p->tasks.size() << "\n"; }
				Task * tt;
				tt = getTaskToSchedule();
				tt->forceCalculating();
				tt->setStatus(TaskStatus::DONE);
				setStatuses();
			}
			
			rcp = p->makespan();
			//std::cout << ".!." << validate2(".!.") << '\n';
			*p = tmp_p1;
		//	for (auto & i : p->resources) std::cout << i.second.points[0] << '\n';
			//std::cout << ".!." << validate2(".!.") << '\n';
			//std::cout << p->resources["R1"].points[0] << '\n';
			//std::cout << "La\n";
			
			
			
			
			/*//rcp = (h->function)(act[0]); //OLD version
			Task* ttt = act[0];
			for (auto &i : act) ttt = bestByHeur(ttt, i);
			rcp = (h->function)(ttt);*/
		}
		//std::cout << "Here\n";
		//std::cout << rcp << '\n';
		return  rcp;
	}
	bool fndMin = (!h) || (!h->findMax);
	float bnd = (fndMin) ? INT32_MAX : 0;
	//Task * tttt = NULL;
	
	 for (auto &i : act)
	{
		 
		i->forceCalculating();
		
		i->setStatus(TaskStatus::DONE);
		//for (auto &k : p->tasks) std::cout << k.second.getName() << '\n';
		setStatuses();
		//std::cout << i->getName() << '\n';
		//std::cout << _depth << '\n';
		/*std::map<Task*, TaskStatus> statuses;

		for (auto &k : p->tasks) { statuses.insert(std::pair<Task*, TaskStatus>(&k.second, k.second.getStatus())); }*/
#ifdef VALIDATE
		//exit(0);
		
		
		validate1(" "+ std::to_string(_depth));
#endif // VALIDATE

		float b = bound(_depth - 1);
		//std::cout << _depth << '\n';
		//for (auto &k : p->tasks) std::cout << k.first << ' ' << k.second.start() << ' ' << k.second.getStatus() << '\n';
		//for (auto &k : p->tasks) if (statuses[&k.second] != k.second.getStatus()) { (&k.second)->setStatus(statuses[&k.second]); /*std::cout << k.second.getName() << " HAS GOT CHANGED from " << statuses[&k.second] << " to " << k.second.getStatus() << '\n';*/ }
#ifdef VALIDATE


		int a;
		if (a = validate2(" " + std::to_string(_depth)) > 0) { /*for (auto &k : p->tasks) std::cout << k.first << ' ' << k.second.start() << ' ' << k.second.getStatus() << '\n';*/ std::cout << _depth << '\n'; std::cout << a << '\n'; exit(0); }
		
#endif // VALIDATE
		setStatuses();
		//std::cout << "La " << _depth << '\n';
		if ((fndMin) ^ (b > bnd)) { bnd = b; }
		//i->setStart(0);
		for (auto & j : (i)->assignments)
		{
			j->resource->deleteTask(j);
		}
		i->setStatus(TaskStatus::ACTIVE);
		//active[i] = true;
		setStatuses();
		for (auto &l = p->tasks.begin(); l != p->tasks.end(); l++) if (l->second.getStatus() != TaskStatus::DONE) l->second.setStart(0);

		//std::cout << "Ici\n";
	}
	//setStatuses();
	//for (auto i : bounds) std::cout << i->getName() << '\n';
	return bnd;
}


float MixedSolver::solve()
{
	//std::cout << "Start\n";
	//for (auto &i : p->tasks) std::cout << i.first << ' ' << i.second.getStatus() << "  ";
	for (auto &x : p->tasks) {
		
		if ((x.second.prev.size() == 0) /* && (statuses[&x->second] == ::PASSIVE)*/) {
			//std::cout << x.second.getName() << '\n';
			(&x.second)->setStatus(TaskStatus::ACTIVE);
		}
		else {
			(&x.second)->setStatus(TaskStatus::PASSIVE);
			//std::cout << x.second.getName() << '\n';
		}
	}
	//std::cout << "Here\n";
	//for (auto &i : p->tasks) std::cout << i.first << ' ' << i.second.getStatus() << "  ";
	int c = 0;

	for (auto &i : p->tasks)
	{
		Task * t_res = NULL;
		if ((i.second.getStatus() == TaskStatus::ACTIVE) && (i.second.noResources()))
		{
			t_res = &i.second;
			//std::cout << "Okay\n";
			c++;
			//setMaxStart();
			//if (maxStart + t->getDuration() > solution.second) return makespan;
			(&i.second)->forceCalculating();
			//if (t->exceeds(solution.second))
			(&i.second)->setStatus(TaskStatus::DONE);
			setStatuses();
			//break;
		}
	}

	while (haveActiveTasks()) {
	/*	for (auto &i : p->tasks) if (i.second.getStatus() == ::ACTIVE) std::cout << i.second.getName() << ' ';
		std::cout << '\n';*/
		c++; //{ if (c > p->tasks.size()) std::cout << "Something went "<< c << "\n"; }
		Task* tt = NULL;
		int actnum = 0; for (auto &i : p->tasks) { if ((&i.second)->getStatus() == ::ACTIVE) { actnum++; if (!tt) tt = &i.second;  } }
		//std::cout <<  tt->getName() << ' '<< tt->getStatus() << '\n';
		//std::cout << actnum << '\n';
	//	if (actnum == 1) std::cout << "THE ONLY " << tt->getName() << '\n';
		std::map<std::string, Resource> tmp_src(p->resources);
		std::map<std::string, Task> tmp_tsc(p->tasks);
		if (actnum > 1)
		{
			bool fndMin = (!h) || (!h->findMax);
			float bestBound = (fndMin) ? INT32_MAX : 0;
			std::map<std::string, Task> tmp_tasks = p->tasks;
			for (auto &i : p->tasks)
			{
				if (i.second.getStatus() != ::ACTIVE) continue;
				(&i.second)->forceCalculating();

				//std::cout << i.first << ' ' << i.second.start() << '\n';
				//std::cout << "Up here\n"; exit(0);
				(&i.second)->setStatus(::DONE);
				setStatuses();
					//std::cout << "Do\n";
				std::map<Task*, TaskStatus> statuses;
				
				for (auto &j : p->tasks) { statuses.insert(std::pair<Task*, TaskStatus>(&j.second, j.second.getStatus())); }
#ifdef VALIDATE
				validate1("  " + std::to_string(depth));
#endif // VALIDATE
				float tmp = bound(depth);
			//	std::cout << tmp << '\n';
#ifdef VALIDATE
				int a;
				//std::cout << "Here\n";
				if (a = validate2("  " + std::to_string(depth)) > 0) { /*for (auto &k : p->tasks) std::cout << k.first << ' ' << k.second.start() << ' ' << k.second.getStatus() << '\n';*/
				std::cout << "validator3 says " << a << '\n';
				exit(0);
				}
				
#endif // VALIDATE
				//std::cout << "Survived\n";
					
				
				//else std::cout << i.first << " stayed with " << i.second.getStatus() << '\n';
				if ((fndMin) ^ (tmp > bestBound)) { bestBound = tmp; tt = &i.second; }
				//std::cout << bestBound << ' ';
				//(&i.second)->setStart(0);
				//std::cout << i.second.start() << '\n';
				for (auto & j : i.second.assignments)
				{
					j->resource->deleteTask(j);
				}
				(&i.second)->setStatus(TaskStatus::ACTIVE);
				//std::cout<< validate2("  " + std::to_string(depth)) << '\n';
				setStatuses();
			
				for (auto &l = p->tasks.begin(); l != p->tasks.end(); l++) if (l->second.getStatus() != TaskStatus::DONE) l->second.setStart(0);
				
			}
		}
		//std::cout << "Ici\n";
		//p->resources = tmp_src;
		//p->tasks = tmp_tsc;
		/*for (auto &r : p->resources)
		{
			for (auto &pt : r.second.points)
				std::cout << r.first << ' ' << pt.first << ' ' << pt.second << '\n';
		}*/
		//std::cout << tt->getName() << ' ' << tt->start() << '\n';
		tt->forceCalculating();
	
	//	std::cout << '\n';

		tt->setStatus(::DONE);
		setStatuses();
	//	for (auto &i : p->tasks) (&i.second)->fictiveStart = (&i.second)->maxPrevFinish();
	//	for (auto &i : p->tasks) if (i.second.getStatus() == ::ACTIVE) std::cout << i.second.start() << ' ' << i.second.fictiveStart << ' ';
		//std::cout << '\n';
		//std::cout << tt->getName() << ' ' << tt->start() << '\n';
		
		/*for (auto &i : p->tasks) std::cout << i.second.getName() << i.second.getStatus();
		std::cout << '\n';*/


	}

	for (auto &x : p->tasks) {
		if (x.second.getStatus() != ::DONE) {
			p->error = true; p->msg += "NOT SOLVED\n";
			p->notsolved = true;
			break;
		}
	}
	solution.second = p->makespan();
	solution.first = p->tasks;
	return p->makespan();
}